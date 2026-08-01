#include "vending_engine_service/impl/vending_engine_service.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "cloud_service/mocks/mock_cloud_service.hpp"
#include "dispenser_service/mocks/mock_dispenser.hpp"
#include "shared_helper/logging/console_logger.hpp"
#include "shared_helper/mocks/fake_clock.hpp"
#include "shared_helper/mocks/mock_machine_state_store.hpp"
#include "shared_helper/mocks/mock_uuid_generator.hpp"

namespace vending::vending_engine_service {
namespace {

using ::testing::_;
using ::testing::AllOf;
using ::testing::Field;
using ::testing::Return;
using ::testing::SaveArg;

constexpr const char* kCardId = "card-1";
constexpr const char* kProductId = "coke_330";
constexpr const char* kTxId = "tx-1";

class VendingEngineServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        ON_CALL(uuidGen, generate()).WillByDefault(Return(kTxId));

        // The dispenser is a mock: we capture whatever callbacks
        // VendingEngineService registers so tests can simulate progress
        // ticks / a dispense finishing without a real dispenser.
        EXPECT_CALL(dispenser, setOnProgress(_)).WillOnce(SaveArg<0>(&onProgress));
        EXPECT_CALL(dispenser, setOnResult(_)).WillOnce(SaveArg<0>(&onResult));

        // Same idea for cloudService's pending-count/online callbacks.
        EXPECT_CALL(cloudService, setOnPendingCountChanged(_)).WillOnce(SaveArg<0>(&onPendingCountChanged));
        EXPECT_CALL(cloudService, setOnOnlineChanged(_)).WillOnce(SaveArg<0>(&onOnlineChanged));

        service = std::make_unique<VendingEngineService>(stateStore, cloudService, dispenser, uuidGen, clock, logger,
                                                           std::chrono::milliseconds{15000});
    }

    void tapCard() { service->onCardTapped(kCardId); }

    shared_helper::logging::ConsoleLogger logger;
    shared_helper::mocks::FakeClock clock;
    ::testing::NiceMock<shared_helper::mocks::MockUuidGenerator> uuidGen;
    ::testing::NiceMock<shared_helper::persistence::mocks::MockMachineStateStore> stateStore;
    ::testing::NiceMock<cloud_service::mocks::MockCloudService> cloudService;
    ::testing::NiceMock<dispenser_service::mocks::MockDispenser> dispenser;

    std::function<void(int)> onProgress;
    std::function<void(bool)> onResult;
    std::function<void(std::size_t)> onPendingCountChanged;
    std::function<void(bool)> onOnlineChanged;

    std::unique_ptr<VendingEngineService> service;
};

TEST_F(VendingEngineServiceTest, StartsIdle) {
    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, CardTapMovesIdleToCardRead) {
    tapCard();
    EXPECT_EQ(service->state(), State::CardRead);
}

TEST_F(VendingEngineServiceTest, ProductSelectionMovesCardReadToDispensingAndTriggersDispense) {
    EXPECT_CALL(dispenser, dispense(std::string(kProductId)));

    tapCard();
    service->onProductSelected(kProductId);

    EXPECT_EQ(service->state(), State::Dispensing);
}

TEST_F(VendingEngineServiceTest, SuccessfulDispenseCompletesAndReturnsToIdle) {
    EXPECT_CALL(dispenser, dispense(_));
    EXPECT_CALL(cloudService, send(AllOf(Field(&shared_helper::TransactionRecord::id, kTxId),
                                          Field(&shared_helper::TransactionRecord::status,
                                                shared_helper::TxStatus::Completed))));

    tapCard();
    service->onProductSelected(kProductId);
    ASSERT_TRUE(onResult);
    onResult(true);

    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, FailedDispenseReportsFailedThenReturnsToIdle) {
    EXPECT_CALL(dispenser, dispense(_));
    EXPECT_CALL(cloudService, send(Field(&shared_helper::TransactionRecord::status,
                                          shared_helper::TxStatus::Failed)));

    tapCard();
    service->onProductSelected(kProductId);
    ASSERT_TRUE(onResult);
    onResult(false);

    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, DispenseProgressForwardsToRegisteredCallback) {
    EXPECT_CALL(dispenser, dispense(_));

    tapCard();
    service->onProductSelected(kProductId);

    std::vector<int> observed;
    service->setOnDispenseProgress([&observed](int p) { observed.push_back(p); });

    ASSERT_TRUE(onProgress);
    onProgress(20);
    onProgress(60);

    EXPECT_THAT(observed, ::testing::ElementsAre(20, 60));
}

TEST_F(VendingEngineServiceTest, SecondCardTapWhileDispensingIsIgnored) {
    EXPECT_CALL(dispenser, dispense(_)).Times(1);

    tapCard();
    service->onProductSelected(kProductId);
    ASSERT_EQ(service->state(), State::Dispensing);

    // A second tap must not start a second transaction while one is
    // physically in flight.
    service->onCardTapped("card-2");
    EXPECT_EQ(service->state(), State::Dispensing);
}

TEST_F(VendingEngineServiceTest, SecondCardTapWhileCardReadIsIgnored) {
    tapCard();
    ASSERT_EQ(service->state(), State::CardRead);

    service->onCardTapped("card-2");
    EXPECT_EQ(service->state(), State::CardRead);
}

TEST_F(VendingEngineServiceTest, ProductSelectionIgnoredWhenIdle) {
    EXPECT_CALL(dispenser, dispense(_)).Times(0);

    service->onProductSelected(kProductId);

    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, SelectionTimeoutReturnsToIdle) {
    EXPECT_CALL(cloudService, send(Field(&shared_helper::TransactionRecord::status,
                                          shared_helper::TxStatus::Failed)));

    tapCard();
    ASSERT_EQ(service->state(), State::CardRead);

    clock.advance(std::chrono::milliseconds{15000});

    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, SelectionTimeoutDoesNotFireIfProductAlreadySelected) {
    EXPECT_CALL(dispenser, dispense(_));
    EXPECT_CALL(cloudService, send(_)).Times(0);

    tapCard();
    service->onProductSelected(kProductId);
    ASSERT_EQ(service->state(), State::Dispensing);

    // The selection timeout was disarmed by onProductSelected(); advancing
    // the clock must not abandon the in-flight dispense.
    clock.advance(std::chrono::milliseconds{15000});

    EXPECT_EQ(service->state(), State::Dispensing);
}

TEST_F(VendingEngineServiceTest, CardTapAfterTimeoutStartsFreshTransaction) {
    tapCard();
    clock.advance(std::chrono::milliseconds{15000});
    ASSERT_EQ(service->state(), State::Idle);

    EXPECT_CALL(uuidGen, generate()).WillOnce(Return("tx-2"));
    service->onCardTapped("card-2");

    EXPECT_EQ(service->state(), State::CardRead);
}

TEST_F(VendingEngineServiceTest, StartWithNoSavedStateStaysIdle) {
    ON_CALL(stateStore, load()).WillByDefault(Return(std::nullopt));
    EXPECT_CALL(cloudService, send(_)).Times(0);

    service->start();

    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, StartRecoversDispensingAsUnknownNeedsReconciliation) {
    shared_helper::persistence::MachineState saved;
    saved.fsmState = "Dispensing";
    saved.activeTransaction = shared_helper::TransactionRecord{.id = "tx-interrupted", .cardId = kCardId, .productId = {}, .createdAt = {}, .updatedAt = {}, .syncedAt = {}};
    ON_CALL(stateStore, load()).WillByDefault(Return(saved));

    EXPECT_CALL(cloudService, send(AllOf(Field(&shared_helper::TransactionRecord::id, std::string("tx-interrupted")),
                                          Field(&shared_helper::TransactionRecord::status,
                                                shared_helper::TxStatus::UnknownNeedsReconciliation))));
    EXPECT_CALL(stateStore, clear());

    service->start();

    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, StartRecoversCardReadAsFailed) {
    shared_helper::persistence::MachineState saved;
    saved.fsmState = "CardRead";
    saved.activeTransaction = shared_helper::TransactionRecord{.id = "tx-interrupted", .cardId = kCardId, .productId = {}, .createdAt = {}, .updatedAt = {}, .syncedAt = {}};
    ON_CALL(stateStore, load()).WillByDefault(Return(saved));

    EXPECT_CALL(cloudService, send(Field(&shared_helper::TransactionRecord::status, shared_helper::TxStatus::Failed)));
    EXPECT_CALL(stateStore, clear());

    service->start();

    EXPECT_EQ(service->state(), State::Idle);
}

TEST_F(VendingEngineServiceTest, ServiceUsableAfterRecovery) {
    shared_helper::persistence::MachineState saved;
    saved.fsmState = "Dispensing";
    saved.activeTransaction = shared_helper::TransactionRecord{.id = "tx-interrupted", .cardId = kCardId, .productId = {}, .createdAt = {}, .updatedAt = {}, .syncedAt = {}};
    ON_CALL(stateStore, load()).WillByDefault(Return(saved));
    service->start();
    ASSERT_EQ(service->state(), State::Idle);

    EXPECT_CALL(uuidGen, generate()).WillOnce(Return("tx-fresh"));
    tapCard();

    EXPECT_EQ(service->state(), State::CardRead);
}

TEST_F(VendingEngineServiceTest, PendingSyncCountForwardsFromCloudService) {
    std::vector<std::size_t> observed;
    service->setOnPendingSyncCountChanged([&observed](std::size_t count) { observed.push_back(count); });

    ASSERT_TRUE(onPendingCountChanged);
    onPendingCountChanged(3);
    onPendingCountChanged(2);

    // The first entry is the replay of the last-known value (0) done by
    // setOnPendingSyncCountChanged() itself, mirroring how state() is
    // replayed via setOnStateChanged().
    EXPECT_THAT(observed, ::testing::ElementsAre(0u, 3u, 2u));
}

TEST_F(VendingEngineServiceTest, OnlineForwardsFromCloudService) {
    std::vector<bool> observed;
    service->setOnOnlineChanged([&observed](bool online) { observed.push_back(online); });

    ASSERT_TRUE(onOnlineChanged);
    onOnlineChanged(false);
    onOnlineChanged(true);

    EXPECT_THAT(observed, ::testing::ElementsAre(true, false, true));
}

}  // namespace
}  // namespace vending::vending_engine_service
