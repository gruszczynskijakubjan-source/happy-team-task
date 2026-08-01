#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "shared_helper/interfaces/transaction_record.hpp"

namespace vending::cloud_service {

class IOutboxStorage {
public:
    virtual ~IOutboxStorage() = default;

    virtual void store(const shared_helper::TransactionRecord& tx) = 0;
    virtual shared_helper::TransactionRecord getOldestRecord() = 0;
    virtual void markSynced(const std::string& id) = 0;
};

}  // namespace vending::cloud_service
