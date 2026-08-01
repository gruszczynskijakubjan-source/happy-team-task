#pragma once

#include <chrono>
#include <mutex>
#include <optional>
#include <string>

#include "cloud_service/interfaces/cloud_service.hpp"
#include "dispenser_service/interfaces/dispenser.hpp"
#include "shared_helper/interfaces/clock.hpp"
#include "shared_helper/interfaces/transaction_record.hpp"
#include "shared_helper/interfaces/uuid_generator.hpp"
#include "shared_helper/logging/logger.hpp"
#include "shared_helper/persistence/machine_state_store.hpp"
#include "vending_engine_service/interfaces/vending_engine_service.hpp"

namespace vending::vending_engine_service {

// See IVendingEngineService for the state diagram. This class owns both
// the FSM and the orchestration of dispenser_service/cloud_service/
// shared_helper around it — kept in one place since nothing else needs the
// FSM in isolation (unlike the old, now-removed, standalone
// VendingStateMachine).
//
// Thread-safety: SystemClock and DummyDispenser fire their callbacks
// (selection timeout, dispense progress/result) from their own background
// threads, not the caller's. Every entry point that touches m_state/
// m_currentTransaction/m_selectionTimeoutHandle takes m_mutex up front, so
// the FSM itself is only ever mutated by one thread at a time regardless
// of which thread the call arrived on. m_mutex is recursive because
// onDispenseResult()/onSelectionTimeout() both call finishTransaction()
// while already holding it.
class VendingEngineService final : public IVendingEngineService {
public:
    static constexpr std::chrono::seconds SelectionTimeout{15};

    VendingEngineService(shared_helper::persistence::IMachineStateStore& stateStore
        , cloud_service::ICloudService& cloudService
        , dispenser_service::IDispenser& dispenser
        , shared_helper::UuidGenerator& uuidGen
        , shared_helper::IClock& clock
        , shared_helper::logging::ILogger& logger
        , std::chrono::milliseconds selectionTimeout = SelectionTimeout
    );

    void start() override;

    void onCardTapped(const std::string& cardId) override;
    void onProductSelected(const std::string& productId) override;
    void onDispenseResult(bool ok) override;

    State state() const override {
        std::lock_guard lock(m_mutex);
        return m_state;
    }

    void setOnDispenseProgress(std::function<void(int)> onDispenseProgress) override;
    void setOnDispenseFinished(std::function<void(bool)> onDispenseFinished) override;
    void setOnStateChanged(std::function<void(State)> onStateChanged) override;

private:
    void transitionTo(State next);
    void armSelectionTimeout();
    void disarmSelectionTimeout();
    void onSelectionTimeout();
    void finishTransaction(shared_helper::TxStatus status);

    mutable std::recursive_mutex m_mutex;

    shared_helper::persistence::IMachineStateStore& m_stateStore;
    cloud_service::ICloudService& m_cloudService;
    dispenser_service::IDispenser& m_dispenser;
    shared_helper::UuidGenerator& m_uuidGen;
    shared_helper::IClock& m_clock;
    shared_helper::logging::ILogger& m_logger;
    std::chrono::milliseconds m_selectionTimeout;

    State m_state = State::Idle;
    std::optional<shared_helper::TransactionRecord> m_currentTransaction;
    shared_helper::TimerHandle m_selectionTimeoutHandle = shared_helper::InvalidTimerHandle;

    std::function<void(int)> m_onDispenseProgress;
    std::function<void(bool)> m_onDispenseFinished;
    std::function<void(State)> m_onStateChanged;
};

}  // namespace vending::vending_engine_service
