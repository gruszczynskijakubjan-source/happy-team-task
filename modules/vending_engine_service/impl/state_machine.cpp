#include "vending_engine_service/impl/state_machine.hpp"

namespace vending::vending_engine_service {

VendingStateMachine::VendingStateMachine(shared_helper::IClock& clock, shared_helper::logging::ILogger& logger
    , std::chrono::milliseconds selectionTimeout)
        : m_clock(clock)
        , m_logger(logger)
        , m_selectionTimeout(selectionTimeout) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "VendingStateMachine()");
    // TODO: implement
}

bool VendingStateMachine::onCardTapped(std::string /*cardId*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "onCardTapped()");
    // TODO: implement
    return false;
}

bool VendingStateMachine::onProductSelected(std::string /*productId*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "onProductSelected()");
    // TODO: implement
    return false;
}

void VendingStateMachine::onDispenseResult(bool /*ok*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "onDispenseResult()");
    // TODO: implement
}

void VendingStateMachine::transitionTo(State /*next*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "transitionTo()");
    // TODO: implemen
}

void VendingStateMachine::armTimeout() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "armTimeout()");
    // TODO: implement
}

void VendingStateMachine::disarmTimeout() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "disarmTimeout()");
    // TODO: implement
}

void VendingStateMachine::onTimeout() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingStateMachine", "onTimeout()");
    // TODO: implement
}

}  // namespace vending::vending_engine_service
