#pragma once

#include <memory>
#include <string>

#include "shared_helper/logging/logger.hpp"
#include "shared_helper/sql/sql_adapter.hpp"

// Forward declaration to keep sqlite3.h out of the public header.
struct sqlite3;

namespace vending::shared_helper::sql {

class SqliteSqlAdapter final : public ISqlAdapter {
public:
    SqliteSqlAdapter(const std::string& dbPath
        , logging::ILogger& logger
    );
    ~SqliteSqlAdapter() override;

    SqliteSqlAdapter(const SqliteSqlAdapter&) = delete;
    SqliteSqlAdapter& operator=(const SqliteSqlAdapter&) = delete;

    std::size_t execute(const std::string& sql, const std::vector<Value>& params = {}) override;
    std::vector<Row> query(const std::string& sql, const std::vector<Value>& params = {}) override;

private:
    struct Sqlite3Deleter {
        void operator()(sqlite3* db) const;
    };
    std::unique_ptr<sqlite3, Sqlite3Deleter> m_db;
    logging::ILogger& m_logger;
};

}  // namespace vending::shared_helper::sql
