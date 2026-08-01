#pragma once

#include <memory>
#include <string>

#include "cloud_service/interfaces/outbox_storage.hpp"
#include "shared_helper/logging/logger.hpp"
#include "shared_helper/sql/sql_adapter.hpp"

namespace vending::cloud_service {

class SqliteOutboxStorage final : public IOutboxStorage {
 public:
    SqliteOutboxStorage(shared_helper::sql::ISqlAdapter& sqlAdapter
        , shared_helper::logging::ILogger& logger
    );

    bool store(const shared_helper::TransactionRecord& tx) override;
    std::optional<shared_helper::TransactionRecord> getOldestUnsynced() override;
    std::size_t countUnsynced() override;
    void markSynced(const std::string& id) override;

 private:
    void ensureSchema();

    shared_helper::sql::ISqlAdapter& m_sqlAdapter;
    shared_helper::logging::ILogger& m_logger;
};

}  // namespace vending::cloud_service
