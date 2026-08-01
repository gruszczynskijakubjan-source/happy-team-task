#pragma once

#include <functional>

#include "shared_helper/interfaces/transaction_record.hpp"

namespace vending::cloud_service {

enum class SendStatus {
    Ok,
    RetryableError,
    FatalError,
};

class ICloudSender {
public:
    virtual ~ICloudSender() = default;

    // Synchronous, blocking send of a single record — CloudService is the
    // one place that calls this, from its own worker thread, so blocking
    // here never stalls the FSM or the UI.
    virtual SendStatus send(const shared_helper::TransactionRecord& tx) = 0;
};

}  // namespace vending::cloud_service
