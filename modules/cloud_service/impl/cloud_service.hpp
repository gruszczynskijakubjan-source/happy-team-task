#pragma once

#include <atomic>
#include <chrono>
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

    bool isOnline() const { return m_online; }
    std::size_t pendingCount() const;

 private:
    void runLoop();

    IOutboxStorage& m_storage;
    ICloudSender& m_sender;
    shared_helper::logging::ILogger& m_logger;

    BackoffPolicy m_policy;
    std::atomic<bool> m_online{true};
};

}  // namespace vending::cloud_service
