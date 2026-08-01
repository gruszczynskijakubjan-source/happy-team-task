#pragma once

#include <gmock/gmock.h>

#include "dispenser_service/interfaces/dispenser.hpp"

namespace vending::dispenser_service::mocks {

class MockDispenser : public IDispenser {
public:
    MOCK_METHOD(void, setOnProgress, (std::function<void(int)> onProgress), (override));
    MOCK_METHOD(void, setOnResult, (std::function<void(bool)> onResult), (override));
    MOCK_METHOD(void, dispense, (std::string productId), (override));
};

}  // namespace vending::dispenser_service::mocks
