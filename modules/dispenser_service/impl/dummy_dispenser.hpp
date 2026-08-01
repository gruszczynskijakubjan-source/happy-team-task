#pragma once

#include <chrono>
#include <functional>

#include "dispenser_service/interfaces/dispenser.hpp"
#include "shared_helper/interfaces/clock.hpp"
#include "shared_helper/logging/logger.hpp"

namespace vending::dispenser_service {

class DummyDispenser final : public IDispenser {
public:
    DummyDispenser(shared_helper::IClock& clock
        , shared_helper::logging::ILogger& logger
        , std::chrono::milliseconds stepDelay = std::chrono::seconds{1});

    void setOnProgress(std::function<void(int)> onProgress) override;
    void setOnResult(std::function<void(bool)> onResult) override;
    void dispense(std::string productId) override;

private:
    static constexpr int MinProgress = 0;
    static constexpr int MaxProgress = 100;

    void tick(int progress);

    shared_helper::IClock& m_clock;
    shared_helper::logging::ILogger& m_logger;
    std::chrono::milliseconds m_stepDelay;
    std::function<void(int)> m_onProgress;
    std::function<void(bool)> m_onResult;
};

}  // namespace vending::dispenser_service
