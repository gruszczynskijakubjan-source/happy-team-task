#include "shared_helper/sql/sqlite_sql_adapter.hpp"

#include <stdexcept>

#include <sqlite3.h>

namespace vending::shared_helper::sql {

namespace {

void bindParams(sqlite3_stmt* stmt, const std::vector<Value>& params) {
    for (std::size_t i = 0; i < params.size(); ++i) {
        const int index = static_cast<int>(i) + 1;
        std::visit(
            [stmt, index](auto&& value) {
                using T = std::decay_t<decltype(value)>;
                if constexpr (std::is_same_v<T, std::nullptr_t>) {
                    sqlite3_bind_null(stmt, index);
                } else if constexpr (std::is_same_v<T, std::int64_t>) {
                    sqlite3_bind_int64(stmt, index, value);
                } else if constexpr (std::is_same_v<T, double>) {
                    sqlite3_bind_double(stmt, index, value);
                } else if constexpr (std::is_same_v<T, std::string>) {
                    sqlite3_bind_text(stmt, index, value.c_str(), static_cast<int>(value.size()), SQLITE_TRANSIENT);
                }
            },
            params[i]);
    }
}

Row readRow(sqlite3_stmt* stmt) {
    Row row;
    const int columnCount = sqlite3_column_count(stmt);
    row.reserve(static_cast<std::size_t>(columnCount));
    for (int i = 0; i < columnCount; ++i) {
        switch (sqlite3_column_type(stmt, i)) {
            case SQLITE_INTEGER:
                row.emplace_back(static_cast<std::int64_t>(sqlite3_column_int64(stmt, i)));
                break;
            case SQLITE_FLOAT:
                row.emplace_back(sqlite3_column_double(stmt, i));
                break;
            case SQLITE_NULL:
                row.emplace_back(nullptr);
                break;
            default: {
                const auto* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
                row.emplace_back(std::string(text ? text : ""));
                break;
            }
        }
    }
    return row;
}

}  // namespace

void SqliteSqlAdapter::Sqlite3Deleter::operator()(sqlite3* db) const {
    sqlite3_close(db);
}

SqliteSqlAdapter::SqliteSqlAdapter(const std::string& dbPath, logging::ILogger& logger) : m_logger(logger) {
    m_logger.log(logging::LogLevel::Trace, "SqliteSqlAdapter", "SqliteSqlAdapter(" + dbPath + ")");

    sqlite3* db = nullptr;
    // SQLITE_OPEN_FULLMUTEX: the connection may be used from both the FSM
    // thread (store(), synchronous) and the CloudService worker thread
    // (getOldestUnsynced()/markSynced()) — SQLite's own serialized mode
    // handles that locking for us rather than requiring an external mutex.
    const int rc = sqlite3_open_v2(dbPath.c_str(), &db,
                                    SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nullptr);
    m_db.reset(db);
    if (rc != SQLITE_OK) {
        const std::string message = db ? sqlite3_errmsg(db) : "unknown error";
        throw std::runtime_error("SqliteSqlAdapter: failed to open " + dbPath + ": " + message);
    }

    // WAL lets the worker thread read while the FSM thread writes (and
    // vice versa) without blocking each other on the common case.
    sqlite3_exec(m_db.get(), "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    sqlite3_exec(m_db.get(), "PRAGMA foreign_keys=ON;", nullptr, nullptr, nullptr);
}

SqliteSqlAdapter::~SqliteSqlAdapter() = default;

std::size_t SqliteSqlAdapter::execute(const std::string& sql, const std::vector<Value>& params) {
    m_logger.log(logging::LogLevel::Trace, "SqliteSqlAdapter", "execute(" + sql + ")");

    sqlite3_stmt* rawStmt = nullptr;
    if (sqlite3_prepare_v2(m_db.get(), sql.c_str(), -1, &rawStmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("SqliteSqlAdapter::execute: prepare failed: " +
                                  std::string(sqlite3_errmsg(m_db.get())));
    }
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt(rawStmt, &sqlite3_finalize);

    bindParams(stmt.get(), params);

    const int rc = sqlite3_step(stmt.get());
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        throw std::runtime_error("SqliteSqlAdapter::execute: step failed: " +
                                  std::string(sqlite3_errmsg(m_db.get())));
    }

    return static_cast<std::size_t>(sqlite3_changes(m_db.get()));
}

std::vector<Row> SqliteSqlAdapter::query(const std::string& sql, const std::vector<Value>& params) {
    m_logger.log(logging::LogLevel::Trace, "SqliteSqlAdapter", "query(" + sql + ")");

    sqlite3_stmt* rawStmt = nullptr;
    if (sqlite3_prepare_v2(m_db.get(), sql.c_str(), -1, &rawStmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("SqliteSqlAdapter::query: prepare failed: " +
                                  std::string(sqlite3_errmsg(m_db.get())));
    }
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt(rawStmt, &sqlite3_finalize);

    bindParams(stmt.get(), params);

    std::vector<Row> rows;
    int rc;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        rows.push_back(readRow(stmt.get()));
    }
    if (rc != SQLITE_DONE) {
        throw std::runtime_error("SqliteSqlAdapter::query: step failed: " +
                                  std::string(sqlite3_errmsg(m_db.get())));
    }

    return rows;
}

}  // namespace vending::shared_helper::sql
