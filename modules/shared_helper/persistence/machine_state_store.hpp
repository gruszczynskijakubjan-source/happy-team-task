#pragma once

#include <optional>
#include <string>

#include "shared_helper/interfaces/transaction_record.hpp"

namespace vending::shared_helper::persistence {

struct MachineState {
    std::string fsmState;
    std::optional<TransactionRecord> activeTransaction;
};

class IMachineStateStore {
public:
    virtual ~IMachineStateStore() = default;

    virtual void save(const MachineState& state) = 0;
    virtual std::optional<MachineState> load() = 0;
    virtual void clear() = 0;
};

}  // namespace vending::shared_helper::persistence
