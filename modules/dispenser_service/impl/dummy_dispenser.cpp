#include "dispenser_service/impl/dummy_dispenser.hpp"

namespace vending::dispenser_service {

DummyDispenser::DummyDispenser(shared_helper::IClock& clock
    , shared_helper::logging::ILogger& logger
    , std::chrono::milliseconds stepDelay)
        : m_clock(clock)
        , m_logger(logger)
        , m_stepDelay(stepDelay) {
}

void DummyDispenser::setOnProgress(std::function<void(int)> onProgress) {
    m_onProgress = std::move(onProgress);
}

void DummyDispenser::setOnResult(std::function<void(bool)> onResult) {
    m_onResult = std::move(onResult);
}

void DummyDispenser::dispense(std::string productId) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "DummyDispenser", "dispense(" + productId + ")");
    tick(MinProgress);
}

void DummyDispenser::tick(int progress) {
    if (m_onProgress) {
        m_onProgress(progress);
    }

    if (progress >= MaxProgress) {
        if (m_onResult) {
            m_onResult(true);
        }
        return;
    }

    constexpr int ProgressStep = 20;
    const int nextProgress = progress + ProgressStep;
    m_clock.scheduleOnce(m_stepDelay, [this, nextProgress] { 
        tick(nextProgress);
    });
}

}  // namespace vending::dispenser_service
