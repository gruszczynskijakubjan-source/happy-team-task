#pragma once

#include <memory>
#include <string>
#include <thread>

#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>

#include "cloud_service/interfaces/cloud_sender.hpp"
#include "shared_helper/logging/logger.hpp"

namespace vending::cloud_service {

class RestTransport final : public ICloudSender {
 public:
    RestTransport(std::string baseUrl, shared_helper::logging::ILogger& logger);
    ~RestTransport() override;

    RestTransport(const RestTransport&) = delete;
    RestTransport& operator=(const RestTransport&) = delete;

    SendStatus send(const shared_helper::TransactionRecord& tx) override;

 private:
    std::string m_baseUrl;
    shared_helper::logging::ILogger& m_logger;
};

}  // namespace vending::cloud_service
