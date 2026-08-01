#include "dispenser_service/impl/dummy_dispenser.hpp"

namespace vending::dispenser_service {

namespace {
constexpr int kProgressStep = 20;
}  // namespace

DummyDispenser::DummyDispenser(shared_helper::IClock& clock, shared_helper::logging::ILogger& logger,
                                std::chrono::milliseconds stepDelay)
    : m_clock(clock), m_logger(logger), m_stepDelay(stepDelay) {}

void DummyDispenser::dispense(std::string productId) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "DummyDispenser", "dispense(" + productId + ")");
    tick(0);
}

void DummyDispenser::tick(int progress) {
    if (onProgress) {
        onProgress(progress);
    }

    if (progress >= 100) {
        if (onDispenseResult) {
            onDispenseResult(true);
        }
        return;
    }

    m_clock.scheduleOnce(m_stepDelay, [this, next = progress + kProgressStep] { tick(next); });
}

}  // namespace vending::dispenser_service
