#pragma once

#include "shared_helper/interfaces/transaction_record.hpp"

namespace vending::cloud_service {

class ICloudService {
public:
    virtual ~ICloudService() = default;
    virtual void send(const shared_helper::TransactionRecord& tx) = 0;
};

}  // namespace vending::cloud_service
