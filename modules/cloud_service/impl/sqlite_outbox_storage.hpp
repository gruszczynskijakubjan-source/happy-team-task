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

    void store(const shared_helper::TransactionRecord& tx) override;
    shared_helper::TransactionRecord getOldestRecord() override;
    void markSynced(const std::string& id) override;

 private:
    shared_helper::sql::ISqlAdapter& m_sqlAdapter;
    shared_helper::logging::ILogger& m_logger;
};

}  // namespace vending::cloud_service
