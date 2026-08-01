#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "cloud_service/interfaces/cloud_sender.hpp"
#include "cloud_service/interfaces/cloud_service.hpp"
#include "cloud_service/interfaces/outbox_storage.hpp"
#include "shared_helper/logging/logger.hpp"

namespace vending::cloud_service {

struct BackoffPolicy {
    std::chrono::seconds initialDelay{2};
    double multiplier = 2.0;
    std::chrono::seconds maxDelay{60};
    double jitterRatio = 0.2;
};

// Owns a dedicated worker thread + queue (backed by IOutboxStorage/SQLite,
// not an in-memory queue — see send()) that syncs transactions to the
// backend independently of the FSM/UI thread.
//
// send() is synchronous but fast: it's a local SQLite INSERT (typically
// sub-millisecond), not a network call, so the FSM thread is never blocked
// waiting on the backend. The actual (slow, network-bound) send happens on
// the worker thread in runLoop().
class CloudService final : public ICloudService {
 public:
    CloudService(IOutboxStorage& storage
        , ICloudSender& sender
        , shared_helper::logging::ILogger& logger
        , BackoffPolicy policy = BackoffPolicy{});
    ~CloudService() override;

    CloudService(const CloudService&) = delete;
    CloudService& operator=(const CloudService&) = delete;

    void send(const shared_helper::TransactionRecord& tx) override;

    void setOnPendingCountChanged(std::function<void(std::size_t)> onPendingCountChanged) override;
    void setOnOnlineChanged(std::function<void(bool)> onOnlineChanged) override;

    bool isOnline() const { return m_online; }
    std::size_t pendingCount() const;

 private:
    void runLoop(std::stop_token stopToken);
    std::chrono::milliseconds nextBackoffDelay();
    void resetBackoff();
    void setPendingCount(std::size_t count);
    void setOnline(bool online);

    IOutboxStorage& m_storage;
    ICloudSender& m_sender;
    shared_helper::logging::ILogger& m_logger;
    BackoffPolicy m_policy;

    std::atomic<bool> m_online{true};

    // Woken by send() (new record available) or by the worker's own
    // backoff timeout, whichever comes first — no polling.
    std::mutex m_wakeMutex;
    std::condition_variable_any m_wakeCv;
    std::chrono::seconds m_currentBackoff;
    std::atomic<std::size_t> m_pendingCount;

    // Guards the two callbacks below: setOnPendingCountChanged()/
    // setOnOnlineChanged() may be called from a different thread (the
    // composition root, at startup) than the one invoking them (the
    // worker thread, or send()'s caller).
    std::mutex m_callbacksMutex;
    std::function<void(std::size_t)> m_onPendingCountChanged;
    std::function<void(bool)> m_onOnlineChanged;

    std::jthread m_worker;
};

}  // namespace vending::cloud_service
