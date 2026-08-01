#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "shared_helper/interfaces/transaction_record.hpp"

namespace vending::cloud_service {

class IOutboxStorage {
public:
    virtual ~IOutboxStorage() = default;

    // Inserts tx and returns true, unless a record with the same id
    // already exists, in which case it's a no-op (implementations should
    // log it) and false is returned — callers may retry store() for the
    // same transaction without producing duplicates, and can tell whether
    // a new record actually landed (e.g. to keep a pending count
    // accurate). Must be synchronous and fast: callers rely on the record
    // being durable by the time store() returns, not on a background
    // write completing later.
    virtual bool store(const shared_helper::TransactionRecord& tx) = 0;

    // Oldest not-yet-synced record, if any.
    virtual std::optional<shared_helper::TransactionRecord> getOldestUnsynced() = 0;

    // Count of not-yet-synced records. Used once, at startup, to seed
    // CloudService's in-memory pending counter with whatever was left in
    // the outbox from a previous run — after that, the counter is
    // maintained incrementally (store()/markSynced() calls), not by
    // re-querying this.
    virtual std::size_t countUnsynced() = 0;

    virtual void markSynced(const std::string& id) = 0;
};

}  // namespace vending::cloud_service
