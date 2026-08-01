#include "cloud_service/impl/sqlite_outbox_storage.hpp"

namespace vending::cloud_service {

SqliteOutboxStorage::SqliteOutboxStorage(shared_helper::sql::ISqlAdapter& sqlAdapter
    , shared_helper::logging::ILogger& logger)
        : m_sqlAdapter(sqlAdapter)
        , m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "SqliteOutboxStorage()");
}

void SqliteOutboxStorage::store(const shared_helper::TransactionRecord& /*tx*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "enqueue()");
    // TODO: implement
}

shared_helper::TransactionRecord SqliteOutboxStorage::getOldestRecord() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "getOldestRecord()");
    // TODO: implement
    return shared_helper::TransactionRecord{};
}

void SqliteOutboxStorage::markSynced(const std::string& /*id*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "markSynced()");
    // TODO: implement
}

}  // namespace vending::cloud_service
