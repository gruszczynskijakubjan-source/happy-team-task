#pragma once

#include <string>

#include "shared_helper/logging/logger.hpp"
#include "shared_helper/persistence/machine_state_store.hpp"

namespace vending::shared_helper::persistence {

class JsonMachineStateStore final : public IMachineStateStore {
public:
    JsonMachineStateStore(std::string path, logging::ILogger& logger);

    void save(const MachineState& state) override;
    std::optional<MachineState> load() override;
    void clear() override;

private:
    std::string m_path;
    logging::ILogger& m_logger;
};

}  // namespace vending::shared_helper::persistence
