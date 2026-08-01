#pragma once

#include <gmock/gmock.h>

#include "dispenser_service/interfaces/dispenser.hpp"

namespace vending::dispenser_service::mocks {

class MockDispenser : public IDispenser {
public:
    MOCK_METHOD(void, dispense, (std::string productId), (override));
};

}  // namespace vending::dispenser_service::mocks
