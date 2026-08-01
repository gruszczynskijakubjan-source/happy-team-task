#pragma once

#include <cstddef>
#include <functional>

#include "shared_helper/interfaces/transaction_record.hpp"

namespace vending::cloud_service {

class ICloudService {
public:
    virtual ~ICloudService() = default;
    virtual void send(const shared_helper::TransactionRecord& tx) = 0;

    // Registered once, up front, by whoever wants to observe outbox/
    // connectivity state (e.g. vending_engine_service, which re-publishes
    // it to the UI). Each fires once immediately with the current value
    // when registered, then again on every subsequent change, so callers
    // don't need a separate initial poll.
    virtual void setOnPendingCountChanged(std::function<void(std::size_t)> onPendingCountChanged) = 0;
    virtual void setOnOnlineChanged(std::function<void(bool)> onOnlineChanged) = 0;
};

}  // namespace vending::cloud_service
