#include "VendingController.hpp"

#include <QMetaObject>

namespace vending::ui_service {

VendingController::VendingController(vending_engine_service::TransactionService& transactionService,
                                      vending_engine_service::VendingStateMachine& fsm,
                                      cloud_service::CloudService& cloudService,
                                      dispenser_service::IDispenser& dispenser,
                                      rfid_service::ICardReader& cardReader,
                                      shared_helper::logging::ILogger& logger, QObject* parent)
    : QObject(parent),
      m_transactionService(transactionService),
      m_fsm(fsm),
      m_cloudService(cloudService),
      m_dispenser(dispenser),
      m_cardReader(cardReader),
      m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingController", "VendingController()");

    // DummyDispenser/DummyCardReader fire these callbacks from their own
    // timer thread; queue back onto this object's (the GUI) thread.
    m_dispenser.onProgress = [this](int progress) {
        QMetaObject::invokeMethod(
            this,
            [this, progress] {
                m_dispenseProgress = progress;
                emit dispenseProgressChanged();
            },
            Qt::QueuedConnection);
    };
    m_dispenser.onDispenseResult = [this](bool /*ok*/) {
        QMetaObject::invokeMethod(
            this,
            [this] {
                m_dispensing = false;
                emit dispensingChanged();
            },
            Qt::QueuedConnection);
    };
}

VendingController::~VendingController() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingController", "~VendingController()");
}

QString VendingController::stateName() const {
    // TODO: implement
    return {};
}

bool VendingController::dispensing() const {
    return m_dispensing;
}

int VendingController::dispenseProgress() const {
    return m_dispenseProgress;
}

bool VendingController::online() const {
    // TODO: implement
    return false;
}

int VendingController::pendingSyncCount() const {
    // TODO: implement
    return 0;
}

void VendingController::simulateCardTap() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingController", "simulateCardTap()");
    m_cardReader.simulateTap();
}

void VendingController::selectProduct(const QString& productId) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingController", "selectProduct()");
    m_dispensing = true;
    m_dispenseProgress = 0;
    emit dispensingChanged();
    emit dispenseProgressChanged();
    m_dispenser.dispense(productId.toStdString());
}

}  // namespace vending::ui_service
