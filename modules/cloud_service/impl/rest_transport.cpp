#include "cloud_service/impl/rest_transport.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace vending::cloud_service {

RestTransport::RestTransport(std::string baseUrl
    , shared_helper::logging::ILogger& logger)
        : m_baseUrl(std::move(baseUrl))
        , m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "RestTransport", "RestTransport()");
    // TODO: implement
}

RestTransport::~RestTransport() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "RestTransport", "~RestTransport()");
    // TODO: implement
}

SendStatus RestTransport::sendSync(const shared_helper::TransactionRecord& /*tx*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "RestTransport", "send()");
    // TODO: implement
}

}  // namespace vending::cloud_service
