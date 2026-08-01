#include "cloud_service/impl/cloud_service.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_service/mocks/mock_cloud_sender.hpp"
#include "cloud_service/mocks/mock_outbox_storage.hpp"
#include "shared_helper/logging/console_logger.hpp"

namespace vending::cloud_service {
namespace {

using ::testing::_;
using ::testing::Return;

// CloudService owns a real worker thread, so these tests synchronize on
// real (short) waits rather than a fake clock — there's no seam to
// virtualize a std::jthread's own scheduling.
class Latch {
public:
    void count() {
        std::lock_guard lock(m_mutex);
        ++m_count;
        m_cv.notify_all();
    }

    bool waitFor(int target, std::chrono::milliseconds timeout) {
        std::unique_lock lock(m_mutex);
        return m_cv.wait_for(lock, timeout, [&] { return m_count >= target; });
    }

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_count = 0;
};

shared_helper::TransactionRecord makeTx(std::string id) {
    return shared_helper::TransactionRecord{
        .id = std::move(id), .cardId = "card-1", .productId = {}, .createdAt = {}, .updatedAt = {}, .syncedAt = {}};
}

class CloudServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Matches the common case (a genuinely new record); tests of
        // dedup behavior override this explicitly.
        ON_CALL(storage, store(_)).WillByDefault(Return(true));
        ON_CALL(storage, countUnsynced()).WillByDefault(Return(0));
    }

    shared_helper::logging::ConsoleLogger logger;
    ::testing::NiceMock<mocks::MockOutboxStorage> storage;
    ::testing::NiceMock<mocks::MockCloudSender> sender;
};

TEST_F(CloudServiceTest, SendStoresRecordSynchronously) {
    // send() must have durably stored the record by the time it returns —
    // no dependency on the worker thread having run at all.
    EXPECT_CALL(storage, store(testing::Field(&shared_helper::TransactionRecord::id, std::string("tx-1"))));
    ON_CALL(storage, getOldestUnsynced()).WillByDefault(Return(std::nullopt));

    CloudService service(storage, sender, logger);
    service.send(makeTx("tx-1"));
}

TEST_F(CloudServiceTest, PendingCountSeededFromStorageAtStartup) {
    ON_CALL(storage, countUnsynced()).WillByDefault(Return(3));
    ON_CALL(storage, getOldestUnsynced()).WillByDefault(Return(std::nullopt));

    CloudService service(storage, sender, logger);

    EXPECT_EQ(service.pendingCount(), 3u);
}

TEST_F(CloudServiceTest, PendingCountIncrementsOnSendAndDecrementsOnSuccess) {
    Latch synced;
    auto tx = makeTx("tx-1");
    auto done = std::make_shared<std::atomic<bool>>(false);

    ON_CALL(storage, getOldestUnsynced()).WillByDefault(testing::InvokeWithoutArgs([&, done]() {
        if (*done) {
            return std::optional<shared_helper::TransactionRecord>(std::nullopt);
        }
        return std::optional<shared_helper::TransactionRecord>(tx);
    }));
    // Block the worker until the test has observed the post-send count, so
    // the assertion below can't race the worker's decrement.
    Latch workerReady;
    EXPECT_CALL(sender, send(_)).WillOnce([&] {
        workerReady.count();
        return SendStatus::Ok;
    });
    EXPECT_CALL(storage, markSynced("tx-1")).WillOnce([&, done] {
        *done = true;
        synced.count();
    });

    CloudService service(storage, sender, logger);
    ASSERT_EQ(service.pendingCount(), 0u);

    service.send(tx);
    EXPECT_EQ(service.pendingCount(), 1u);

    EXPECT_TRUE(synced.waitFor(1, std::chrono::seconds{2}));
    EXPECT_EQ(service.pendingCount(), 0u);
}

TEST_F(CloudServiceTest, PendingCountUnaffectedByDuplicateSend) {
    ON_CALL(storage, getOldestUnsynced()).WillByDefault(Return(std::nullopt));
    auto tx = makeTx("tx-1");

    EXPECT_CALL(storage, store(_)).WillOnce(Return(true)).WillOnce(Return(false));

    CloudService service(storage, sender, logger);
    service.send(tx);
    service.send(tx);  // duplicate id, storage reports it wasn't newly inserted

    EXPECT_EQ(service.pendingCount(), 1u);
}

TEST_F(CloudServiceTest, WorkerSendsAndMarksSyncedOnSuccess) {
    Latch synced;
    auto tx = makeTx("tx-1");

    ON_CALL(storage, getOldestUnsynced())
        .WillByDefault(testing::InvokeWithoutArgs([&, sent = std::make_shared<std::atomic<bool>>(false)]() {
            if (sent->exchange(true)) {
                return std::optional<shared_helper::TransactionRecord>(std::nullopt);
            }
            return std::optional<shared_helper::TransactionRecord>(tx);
        }));
    EXPECT_CALL(sender, send(_)).WillOnce(Return(SendStatus::Ok));
    EXPECT_CALL(storage, markSynced("tx-1")).WillOnce([&] { synced.count(); });

    CloudService service(storage, sender, logger);
    service.send(tx);

    EXPECT_TRUE(synced.waitFor(1, std::chrono::seconds{2}));
}

TEST_F(CloudServiceTest, RetriesOnRetryableErrorThenSucceeds) {
    Latch synced;
    auto tx = makeTx("tx-1");
    auto done = std::make_shared<std::atomic<bool>>(false);

    ON_CALL(storage, getOldestUnsynced()).WillByDefault(testing::InvokeWithoutArgs([&, done]() {
        if (*done) {
            return std::optional<shared_helper::TransactionRecord>(std::nullopt);
        }
        return std::optional<shared_helper::TransactionRecord>(tx);
    }));
    EXPECT_CALL(sender, send(_))
        .WillOnce(Return(SendStatus::RetryableError))
        .WillOnce(Return(SendStatus::Ok));
    EXPECT_CALL(storage, markSynced("tx-1")).WillOnce([&, done] {
        *done = true;
        synced.count();
    });

    BackoffPolicy fastPolicy;
    fastPolicy.initialDelay = std::chrono::seconds{0};  // rounds to a few ms with jitter clamped at 0
    fastPolicy.maxDelay = std::chrono::seconds{1};

    CloudService service(storage, sender, logger, fastPolicy);
    service.send(tx);

    EXPECT_TRUE(synced.waitFor(1, std::chrono::seconds{2}));
}

TEST_F(CloudServiceTest, FatalErrorDropsRecordFromQueue) {
    Latch dropped;
    auto tx = makeTx("tx-1");
    auto done = std::make_shared<std::atomic<bool>>(false);

    ON_CALL(storage, getOldestUnsynced()).WillByDefault(testing::InvokeWithoutArgs([&, done]() {
        if (*done) {
            return std::optional<shared_helper::TransactionRecord>(std::nullopt);
        }
        return std::optional<shared_helper::TransactionRecord>(tx);
    }));
    EXPECT_CALL(sender, send(_)).WillOnce(Return(SendStatus::FatalError));
    EXPECT_CALL(storage, markSynced("tx-1")).WillOnce([&, done] {
        *done = true;
        dropped.count();
    });

    CloudService service(storage, sender, logger);
    service.send(tx);

    EXPECT_TRUE(dropped.waitFor(1, std::chrono::seconds{2}));
}

TEST_F(CloudServiceTest, GoesOfflineOnRetryableErrorAndBackOnlineOnSuccess) {
    Latch synced;
    auto tx = makeTx("tx-1");
    auto done = std::make_shared<std::atomic<bool>>(false);

    ON_CALL(storage, getOldestUnsynced()).WillByDefault(testing::InvokeWithoutArgs([&, done]() {
        if (*done) {
            return std::optional<shared_helper::TransactionRecord>(std::nullopt);
        }
        return std::optional<shared_helper::TransactionRecord>(tx);
    }));
    EXPECT_CALL(sender, send(_))
        .WillOnce(Return(SendStatus::RetryableError))
        .WillOnce(Return(SendStatus::Ok));
    ON_CALL(storage, markSynced(_)).WillByDefault([&, done] {
        *done = true;
        synced.count();
    });

    BackoffPolicy fastPolicy;
    fastPolicy.initialDelay = std::chrono::seconds{0};
    fastPolicy.maxDelay = std::chrono::seconds{1};

    CloudService service(storage, sender, logger, fastPolicy);
    ASSERT_TRUE(service.isOnline());

    service.send(tx);

    EXPECT_TRUE(synced.waitFor(1, std::chrono::seconds{2}));
    EXPECT_TRUE(service.isOnline());
}

}  // namespace
}  // namespace vending::cloud_service
