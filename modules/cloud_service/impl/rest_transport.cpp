#include "cloud_service/impl/rest_transport.hpp"

#include <sstream>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace vending::cloud_service {

namespace {

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

std::string toString(shared_helper::TxStatus status) {
    switch (status) {
        case shared_helper::TxStatus::Pending:
            return "Pending";
        case shared_helper::TxStatus::Selected:
            return "Selected";
        case shared_helper::TxStatus::Dispensing:
            return "Dispensing";
        case shared_helper::TxStatus::Completed:
            return "Completed";
        case shared_helper::TxStatus::Failed:
            return "Failed";
        case shared_helper::TxStatus::UnknownNeedsReconciliation:
            return "UnknownNeedsReconciliation";
    }
    return "UnknownNeedsReconciliation";
}

std::string toJsonBody(const shared_helper::TransactionRecord& tx) {
    std::ostringstream body;
    body << "{"
         << R"("id":")" << tx.id << "\","
         << R"("cardId":")" << tx.cardId << "\","
         << R"("productId":)" << (tx.productId ? "\"" + *tx.productId + "\"" : "null") << ","
         << R"("status":")" << toString(tx.status) << "\""
         << "}";
    return body.str();
}

// baseUrl is expected as "[http://]host[:port]"; the path is fixed
// (POST /transactions per DECISIONS.md).
std::pair<std::string, std::string> splitHostPort(const std::string& baseUrl) {
    std::string hostPort = baseUrl;
    if (const auto schemeEnd = hostPort.find("://"); schemeEnd != std::string::npos) {
        hostPort = hostPort.substr(schemeEnd + 3);
    }
    if (const auto pathStart = hostPort.find('/'); pathStart != std::string::npos) {
        hostPort = hostPort.substr(0, pathStart);
    }

    const auto colon = hostPort.find(':');
    if (colon == std::string::npos) {
        return {hostPort, "80"};
    }
    return {hostPort.substr(0, colon), hostPort.substr(colon + 1)};
}

}  // namespace

RestTransport::RestTransport(std::string baseUrl
    , shared_helper::logging::ILogger& logger)
        : m_baseUrl(std::move(baseUrl))
        , m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "RestTransport", "RestTransport(" + m_baseUrl + ")");
}

RestTransport::~RestTransport() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "RestTransport", "~RestTransport()");
}

SendStatus RestTransport::send(const shared_helper::TransactionRecord& tx) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "RestTransport", "send(" + tx.id + ")");

    // Blocking, synchronous POST — called only from CloudService's worker
    // thread (see CloudService::runLoop), never from the FSM/UI thread.
    try {
        const auto [host, port] = splitHostPort(m_baseUrl);

        asio::io_context ioc;
        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        const auto endpoints = resolver.resolve(host, port);
        stream.expires_after(std::chrono::seconds(5));
        stream.connect(endpoints);

        const std::string body = toJsonBody(tx);
        http::request<http::string_body> req(http::verb::post, "/transactions", 11);
        req.set(http::field::host, host);
        req.set(http::field::content_type, "application/json");
        req.body() = body;
        req.prepare_payload();

        stream.expires_after(std::chrono::seconds(5));
        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        const int statusCode = static_cast<int>(res.result_int());
        if (statusCode >= 200 && statusCode < 300) {
            return SendStatus::Ok;
        }
        if (statusCode >= 500 || statusCode == 429) {
            m_logger.log(shared_helper::logging::LogLevel::Warn, "RestTransport",
                         "send(" + tx.id + ") retryable HTTP " + std::to_string(statusCode));
            return SendStatus::RetryableError;
        }
        m_logger.log(shared_helper::logging::LogLevel::Error, "RestTransport",
                     "send(" + tx.id + ") fatal HTTP " + std::to_string(statusCode));
        return SendStatus::FatalError;
    } catch (const std::exception& e) {
        // Connection refused/timeout/DNS failure — the backend or network
        // is unreachable, which is exactly the retryable case.
        m_logger.log(shared_helper::logging::LogLevel::Warn, "RestTransport",
                     "send(" + tx.id + ") failed: " + std::string(e.what()));
        return SendStatus::RetryableError;
    }
}

}  // namespace vending::cloud_service
