#pragma once

#include <gmock/gmock.h>

#include "cloud_service/interfaces/cloud_service.hpp"

namespace vending::cloud_service::mocks {

class MockCloudService : public ICloudService {
public:
    MOCK_METHOD(void, send, (const shared_helper::TransactionRecord& tx), (override));
};

}  // namespace vending::cloud_service::mocks
