#pragma once

#include <chrono>
#include <functional>
#include <optional>
#include <string>

#include "shared_helper/interfaces/clock.hpp"
#include "shared_helper/logging/logger.hpp"

namespace vending::vending_engine_service {

enum class State {
    Idle,
    CardRead,
    ProductSelected,
    Dispensing,
    Completed,
    Failed,
};

class VendingStateMachine {
public:
    static constexpr std::chrono::seconds kSelectionTimeout{15};

    VendingStateMachine(shared_helper::IClock& clock, shared_helper::logging::ILogger& logger,
                         std::chrono::milliseconds selectionTimeout = kSelectionTimeout);

    // Returns false if the tap was rejected (not in Idle).
    bool onCardTapped(std::string cardId);
    bool onProductSelected(std::string productId);
    void onDispenseResult(bool ok);

    State state() const { return m_state; }
    const std::optional<std::string>& cardId() const { return m_cardId; }
    const std::optional<std::string>& productId() const { return m_productId; }

    // Fired whenever the state changes; useful for wiring into a Qt facade
    // without the state machine depending on Qt itself.
    std::function<void(State)> onStateChanged;

private:
    void transitionTo(State next);
    void armTimeout();
    void disarmTimeout();
    void onTimeout();

    shared_helper::IClock& m_clock;
    shared_helper::logging::ILogger& m_logger;
    std::chrono::milliseconds m_selectionTimeout;
    State m_state = State::Idle;
    std::optional<std::string> m_cardId;
    std::optional<std::string> m_productId;
};

}  // namespace vending::vending_engine_service
