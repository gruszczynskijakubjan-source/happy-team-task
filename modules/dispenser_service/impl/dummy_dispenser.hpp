#pragma once

#include <chrono>

#include "dispenser_service/interfaces/dispenser.hpp"
#include "shared_helper/interfaces/clock.hpp"
#include "shared_helper/logging/logger.hpp"

namespace vending::dispenser_service {

// No real hardware driver exists yet. Simulates a dispense cycle by ticking
// onProgress from 0 to 100 over a few steps, driven by IClock::scheduleOnce,
// before firing onDispenseResult(true).
class DummyDispenser final : public IDispenser {
public:
    DummyDispenser(shared_helper::IClock& clock, shared_helper::logging::ILogger& logger,
                   std::chrono::milliseconds stepDelay = std::chrono::milliseconds{300});

    void dispense(std::string productId) override;

private:
    void tick(int progress);

    shared_helper::IClock& m_clock;
    shared_helper::logging::ILogger& m_logger;
    std::chrono::milliseconds m_stepDelay;
};

}  // namespace vending::dispenser_service
