#include "cloud_service/impl/cloud_service.hpp"

namespace vending::cloud_service {

CloudService::CloudService(IOutboxStorage& storage
    , ICloudSender& sender
    , shared_helper::logging::ILogger& logger
    , BackoffPolicy policy)
        : m_storage(storage), m_sender(sender), m_logger(logger), m_policy(policy) {
        m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "CloudService()");
    // TODO: implement — start m_workerThread running runLoop()
}

CloudService::~CloudService() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "~CloudService()");
    // TODO: implement — m_stopping = true, join m_workerThread
}

void CloudService::send(const shared_helper::TransactionRecord& tx) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "send()");
    // TODO: implement
}

std::size_t CloudService::pendingCount() const {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "pendingCount()");
    // TODO: implement
    return 0;
}

void CloudService::runLoop() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "CloudService", "runLoop()");
    // TODO: implement
}

}  // namespace vending::cloud_service
