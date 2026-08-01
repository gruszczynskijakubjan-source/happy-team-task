#include "vending_engine_service/impl/vending_engine_service.hpp"

namespace vending::vending_engine_service {

namespace {

const char* stateName(State state) {
    switch (state) {
        case State::Idle:
            return "Idle";
        case State::CardRead:
            return "CardRead";
        case State::ProductSelected:
            return "ProductSelected";
        case State::Dispensing:
            return "Dispensing";
        case State::Completed:
            return "Completed";
        case State::Failed:
            return "Failed";
    }
    return "Unknown";
}

}  // namespace

VendingEngineService::VendingEngineService(shared_helper::persistence::IMachineStateStore& stateStore
    , cloud_service::ICloudService& cloudService
    , dispenser_service::IDispenser& dispenser
    , shared_helper::UuidGenerator& uuidGen
    , shared_helper::IClock& clock
    , shared_helper::logging::ILogger& logger
    , std::chrono::milliseconds selectionTimeout)
        : m_stateStore(stateStore)
        , m_cloudService(cloudService)
        , m_dispenser(dispenser)
        , m_uuidGen(uuidGen)
        , m_clock(clock)
        , m_logger(logger)
        , m_selectionTimeout(selectionTimeout) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "VendingEngineService()");

    m_dispenser.setOnProgress([this](int progress) {
        if (m_onDispenseProgress) {
            m_onDispenseProgress(progress);
        }
    });
    m_dispenser.setOnResult([this](bool ok) { onDispenseResult(ok); });
}

void VendingEngineService::start() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "start()");
    // TODO: recover m_currentTransaction/m_state from m_stateStore.load()
    // after a crash mid-dispense.
}

void VendingEngineService::onCardTapped(const std::string& cardId) {
    std::lock_guard lock(m_mutex);

    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "onCardTapped(" + cardId + ")");

    if (m_state != State::Idle) {
        m_logger.log(shared_helper::logging::LogLevel::Warn, "VendingEngineService",
                     "onCardTapped() ignored, not Idle (state=" + std::string(stateName(m_state)) + ")");
        return;
    }

    const auto now = m_clock.now();
    m_currentTransaction = shared_helper::TransactionRecord{
        .id = m_uuidGen.generate(),
        .cardId = cardId,
        .productId = {},
        .status = shared_helper::TxStatus::Pending,
        .createdAt = now,
        .updatedAt = now,
        .syncedAt = {},
    };

    // Write-before-dispense: persisted now so a crash before dispensing
    // starts can still be recovered/reconciled on restart.
    m_stateStore.save({stateName(State::CardRead), m_currentTransaction});

    transitionTo(State::CardRead);
    armSelectionTimeout();
}

void VendingEngineService::onProductSelected(const std::string& productId) {
    std::lock_guard lock(m_mutex);

    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService",
                 "onProductSelected(" + productId + ")");

    if (m_state != State::CardRead) {
        m_logger.log(shared_helper::logging::LogLevel::Warn, "VendingEngineService",
                     "onProductSelected() ignored, not CardRead (state=" + std::string(stateName(m_state)) + ")");
        return;
    }

    disarmSelectionTimeout();

    m_currentTransaction->productId = productId;
    m_currentTransaction->status = shared_helper::TxStatus::Selected;
    m_currentTransaction->updatedAt = m_clock.now();
    m_stateStore.save({stateName(State::ProductSelected), m_currentTransaction});

    transitionTo(State::ProductSelected);
    transitionTo(State::Dispensing);
    m_dispenser.dispense(productId);
}

void VendingEngineService::onDispenseResult(bool ok) {
    std::lock_guard lock(m_mutex);

    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "onDispenseResult(" +
                 std::string(ok ? "true" : "false") + ")");

    if (m_state != State::Dispensing) {
        m_logger.log(shared_helper::logging::LogLevel::Warn, "VendingEngineService",
                     "onDispenseResult() ignored, not Dispensing (state=" + std::string(stateName(m_state)) + ")");
        return;
    }

    finishTransaction(ok ? shared_helper::TxStatus::Completed : shared_helper::TxStatus::Failed);
    transitionTo(ok ? State::Completed : State::Failed);

    if (m_onDispenseFinished) {
        m_onDispenseFinished(ok);
    }

    transitionTo(State::Idle);
}

void VendingEngineService::onSelectionTimeout() {
    std::lock_guard lock(m_mutex);

    m_logger.log(shared_helper::logging::LogLevel::Info, "VendingEngineService",
                 "selection timeout, abandoning transaction");

    if (m_state != State::CardRead) {
        // Already progressed (product selected) or reset; nothing to do.
        return;
    }

    m_selectionTimeoutHandle = shared_helper::InvalidTimerHandle;
    finishTransaction(shared_helper::TxStatus::Failed);
    transitionTo(State::Failed);
    transitionTo(State::Idle);
}

void VendingEngineService::finishTransaction(shared_helper::TxStatus status) {
    if (!m_currentTransaction) {
        return;
    }

    m_currentTransaction->status = status;
    m_currentTransaction->updatedAt = m_clock.now();
    m_cloudService.send(*m_currentTransaction);

    m_stateStore.clear();
    m_currentTransaction.reset();
}

void VendingEngineService::transitionTo(State next) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService",
                 std::string("transitionTo(") + stateName(next) + ")");
    m_state = next;
    if (m_onStateChanged) {
        m_onStateChanged(next);
    }
}

void VendingEngineService::armSelectionTimeout() {
    m_selectionTimeoutHandle = m_clock.scheduleOnce(m_selectionTimeout, [this] {
        onSelectionTimeout();
    });
}

void VendingEngineService::disarmSelectionTimeout() {
    m_clock.cancel(m_selectionTimeoutHandle);
    m_selectionTimeoutHandle = shared_helper::InvalidTimerHandle;
}

void VendingEngineService::setOnDispenseProgress(std::function<void(int)> onDispenseProgress) {
    m_onDispenseProgress = std::move(onDispenseProgress);
}

void VendingEngineService::setOnDispenseFinished(std::function<void(bool)> onDispenseFinished) {
    m_onDispenseFinished = std::move(onDispenseFinished);
}

void VendingEngineService::setOnStateChanged(std::function<void(State)> onStateChanged) {
    m_onStateChanged = std::move(onStateChanged);
}

}  // namespace vending::vending_engine_service
