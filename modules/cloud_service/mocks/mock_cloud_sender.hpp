#pragma once

#include <gmock/gmock.h>

#include "cloud_service/interfaces/cloud_sender.hpp"

namespace vending::cloud_service::mocks {

class MockCloudSender : public ICloudSender {
public:
    MOCK_METHOD(SendStatus, send, (const shared_helper::TransactionRecord& tx), (override));
};

}  // namespace vending::cloud_service::mocks
