#include "shared_helper/sql/sqlite_sql_adapter.hpp"

#include <sqlite3.h>

namespace vending::shared_helper::sql {

void SqliteSqlAdapter::Sqlite3Deleter::operator()(sqlite3* db) const {
    // TODO: implement — sqlite3_close(db)
    (void)db;
}

SqliteSqlAdapter::SqliteSqlAdapter(const std::string& /*dbPath*/, logging::ILogger& logger) : m_logger(logger) {
    m_logger.log(logging::LogLevel::Trace, "SqliteSqlAdapter", "SqliteSqlAdapter()");
    // TODO: implement — sqlite3_open
}

SqliteSqlAdapter::~SqliteSqlAdapter() = default;

std::size_t SqliteSqlAdapter::execute(const std::string& /*sql*/, const std::vector<Value>& /*params*/) {
    m_logger.log(logging::LogLevel::Trace, "SqliteSqlAdapter", "execute()");
    // TODO: implement
    return 0;
}

std::vector<Row> SqliteSqlAdapter::query(const std::string& /*sql*/, const std::vector<Value>& /*params*/) {
    m_logger.log(logging::LogLevel::Trace, "SqliteSqlAdapter", "query()");
    // TODO: implement
    return {};
}

}  // namespace vending::shared_helper::sql
