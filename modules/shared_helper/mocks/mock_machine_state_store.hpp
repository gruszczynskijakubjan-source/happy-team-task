#pragma once

#include <gmock/gmock.h>

#include "shared_helper/persistence/machine_state_store.hpp"

namespace vending::shared_helper::persistence::mocks {

class MockMachineStateStore : public IMachineStateStore {
public:
    MOCK_METHOD(void, save, (const MachineState& state), (override));
    MOCK_METHOD(std::optional<MachineState>, load, (), (override));
    MOCK_METHOD(void, clear, (), (override));
};

}  // namespace vending::shared_helper::persistence::mocks
