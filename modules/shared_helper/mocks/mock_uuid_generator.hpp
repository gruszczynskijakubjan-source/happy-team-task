#pragma once

#include <gmock/gmock.h>

#include "shared_helper/interfaces/uuid_generator.hpp"

namespace vending::shared_helper::mocks {

class MockUuidGenerator : public UuidGenerator {
public:
    MOCK_METHOD(std::string, generate, (), (override));
};

}  // namespace vending::shared_helper::mocks
