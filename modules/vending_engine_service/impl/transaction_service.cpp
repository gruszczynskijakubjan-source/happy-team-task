#include "vending_engine_service/interfaces/transaction_service.hpp"

namespace vending::vending_engine_service {

TransactionService::TransactionService(VendingStateMachine& fsm
    , shared_helper::persistence::IMachineStateStore& stateStore
    , cloud_service::ICloudService& cloudService
    , dispenser_service::IDispenser& dispenser
    , rfid_service::ICardReader& cardReader
    , shared_helper::UuidGenerator& uuidGen
    , shared_helper::logging::ILogger& logger)
        : m_fsm(fsm)
        , m_stateStore(stateStore)
        , m_cloudService(cloudService)
        , m_dispenser(dispenser)
        , m_cardReader(cardReader)
        , m_uuidGen(uuidGen)
        , m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "TransactionService", "TransactionService()");
    // TODO: implement
    // m_currentTransactionId/m_fsm state after a restart
}

void TransactionService::start() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "TransactionService", "start()");
    // TODO: implement
}

void TransactionService::onCardTapped(const std::string& /*cardId*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "TransactionService", "onCardTapped()");
    // TODO: implement
}

void TransactionService::onProductSelected(const std::string& /*productId*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "TransactionService", "onProductSelected()");
    // TODO: implement
}

void TransactionService::onDispenseResult(bool /*ok*/) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "TransactionService", "onDispenseResult()");
    // TODO: implement
    // then m_cloudService.send(tx) once terminal, then m_stateStore.clear()
}

}  // namespace vending::vending_engine_service
