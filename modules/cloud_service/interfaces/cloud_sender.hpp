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
    virtual void send(const shared_helper::TransactionRecord& tx) = 0;
};

}  // namespace vending::cloud_service
