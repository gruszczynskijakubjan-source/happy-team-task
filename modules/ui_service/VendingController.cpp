#include "VendingController.hpp"

#include <QMetaObject>

namespace vending::ui_service {

VendingController::VendingController(vending_engine_service::IVendingEngineService& vendingEngineService
    , cloud_service::CloudService& cloudService
    , rfid_service::ICardReader& cardReader
    , shared_helper::logging::ILogger& logger, QObject* parent)
    : QObject(parent)
    , m_vendingEngineService(vendingEngineService)
    , m_cloudService(cloudService)
    , m_cardReader(cardReader)
    , m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingController", "VendingController()");

    m_vendingEngineService.setOnDispenseProgress([this](int progress) {
        QMetaObject::invokeMethod(
            this,
            [this, progress] {
                m_dispenseProgress = progress;
                emit dispenseProgressChanged();
            },
            Qt::QueuedConnection);
    });
    m_vendingEngineService.setOnDispenseFinished([this](bool /*ok*/) {
        QMetaObject::invokeMethod(
            this,
            [this] {
                m_dispensing = false;
                emit dispensingChanged();
            },
            Qt::QueuedConnection);
    });
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
    m_vendingEngineService.onProductSelected(productId.toStdString());
}

}  // namespace vending::ui_service
