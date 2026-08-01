#include "cloud_service/impl/cloud_service.hpp"

#include <random>

namespace vending::cloud_service {

namespace {

std::mt19937& rng() {
    static thread_local std::mt19937 engine{std::random_device{}()};
    return engine;
}

}  // namespace

CloudService::CloudService(IOutboxStorage& storage
    , ICloudSender& sender
    , shared_helper::logging::ILogger& logger
    , BackoffPolicy policy)
        : m_storage(storage)
        , m_sender(sender)
        , m_logger(logger)
        , m_policy(policy)
        , m_currentBackoff(policy.initialDelay)
        , m_pendingCount(storage.countUnsynced()) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService",
                 "CloudService() pendingCount=" + std::to_string(m_pendingCount.load()));
    m_worker = std::jthread([this](std::stop_token stopToken) {
        runLoop(stopToken);
    });
}

CloudService::~CloudService() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "~CloudService()");
    m_worker.request_stop();
    m_wakeCv.notify_all();
}

void CloudService::send(const shared_helper::TransactionRecord& tx) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "send(" + tx.id + ")");

    // Only a genuinely new record grows the pending count — a duplicate
    // send() for an id already in the outbox doesn't add a second entry
    // to wait for.
    if (m_storage.store(tx)) {
        setPendingCount(++m_pendingCount);
    }

    m_wakeCv.notify_one();
}

std::size_t CloudService::pendingCount() const {
    return m_pendingCount;
}

void CloudService::setOnPendingCountChanged(std::function<void(std::size_t)> onPendingCountChanged) {
    std::lock_guard lock(m_callbacksMutex);
    m_onPendingCountChanged = std::move(onPendingCountChanged);
    if (m_onPendingCountChanged) {
        m_onPendingCountChanged(m_pendingCount);
    }
}

void CloudService::setOnOnlineChanged(std::function<void(bool)> onOnlineChanged) {
    std::lock_guard lock(m_callbacksMutex);
    m_onOnlineChanged = std::move(onOnlineChanged);
    if (m_onOnlineChanged) {
        m_onOnlineChanged(m_online);
    }
}

void CloudService::setPendingCount(std::size_t count) {
    std::lock_guard lock(m_callbacksMutex);
    if (m_onPendingCountChanged) {
        m_onPendingCountChanged(count);
    }
}

void CloudService::setOnline(bool online) {
    const bool changed = m_online.exchange(online) != online;
    if (!changed) {
        return;
    }
    std::lock_guard lock(m_callbacksMutex);
    if (m_onOnlineChanged) {
        m_onOnlineChanged(online);
    }
}

void CloudService::runLoop(std::stop_token stopToken) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "runLoop()");

    while (!stopToken.stop_requested()) {
        std::optional<shared_helper::TransactionRecord> tx = m_storage.getOldestUnsynced();

        if (!tx) {
            std::unique_lock lock(m_wakeMutex);
            m_wakeCv.wait(lock, stopToken, [this] {
                return false;
            });
            continue;
        }

        const SendStatus status = m_sender.send(*tx);

        if (status == SendStatus::Ok) {
            m_storage.markSynced(tx->id);
            setPendingCount(--m_pendingCount);
            setOnline(true);
            resetBackoff();
            continue;
        }

        if (status == SendStatus::FatalError) {
            m_logger.log(shared_helper::logging::LogLevel::Error, "CloudService",
                         "send(" + tx->id + ") fatal, dropping from queue");
            m_storage.markSynced(tx->id);
            setPendingCount(--m_pendingCount);
            continue;
        }

        setOnline(false);
        const std::chrono::milliseconds delay = nextBackoffDelay();
        m_logger.log(shared_helper::logging::LogLevel::Warn, "CloudService",
                     "send(" + tx->id + ") failed, retrying in " +
                         std::to_string(delay.count()) + "ms");

        std::unique_lock lock(m_wakeMutex);
        m_wakeCv.wait_for(lock, stopToken, delay, [] {
            return false;
        });
    }
}

std::chrono::milliseconds CloudService::nextBackoffDelay() {
    static constexpr double MillisecondsPerSecond = 1000.0;

    const auto jitterRange = static_cast<double>(m_currentBackoff.count()) * m_policy.jitterRatio;
    std::uniform_real_distribution<double> jitterDist(-jitterRange, jitterRange);
    const double jitteredSeconds = static_cast<double>(m_currentBackoff.count()) + jitterDist(rng());
    const auto delay = std::chrono::milliseconds(static_cast<std::int64_t>(jitteredSeconds * MillisecondsPerSecond));

    const auto nextBackoff = std::chrono::seconds(static_cast<std::int64_t>(static_cast<double>(m_currentBackoff.count()) * m_policy.multiplier));
    m_currentBackoff = std::min(nextBackoff, m_policy.maxDelay);

    return std::max(delay, std::chrono::milliseconds{0});
}

void CloudService::resetBackoff() {
    m_currentBackoff = m_policy.initialDelay;
}

}  // namespace vending::cloud_service
