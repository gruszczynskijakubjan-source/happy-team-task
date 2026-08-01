#include "VendingController.hpp"

#include <QMetaObject>

namespace vending::ui_service {

namespace {

QString toQString(vending_engine_service::State state) {
    switch (state) {
        case vending_engine_service::State::Idle:
            return QStringLiteral("Idle");
        case vending_engine_service::State::CardRead:
            return QStringLiteral("CardRead");
        case vending_engine_service::State::ProductSelected:
            return QStringLiteral("ProductSelected");
        case vending_engine_service::State::Dispensing:
            return QStringLiteral("Dispensing");
        case vending_engine_service::State::Completed:
            return QStringLiteral("Completed");
        case vending_engine_service::State::Failed:
            return QStringLiteral("Failed");
    }
    return QStringLiteral("Unknown");
}

}  // namespace

VendingController::VendingController(vending_engine_service::IVendingEngineService& vendingEngineService
    , cloud_service::CloudService& cloudService
    , rfid_service::ICardReader& cardReader
    , shared_helper::logging::ILogger& logger, QObject* parent)
    : QObject(parent)
    , m_vendingEngineService(vendingEngineService)
    , m_cloudService(cloudService)
    , m_cardReader(cardReader)
    , m_logger(logger)
    , m_stateName(toQString(vendingEngineService.state())) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingController", "VendingController()");

    m_vendingEngineService.setOnStateChanged([this](vending_engine_service::State state) {
        QMetaObject::invokeMethod(
            this,
            [this, state] {
                m_stateName = toQString(state);
                emit stateChanged();
            },
            Qt::QueuedConnection);
    });
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
    m_vendingEngineService.setOnPendingSyncCountChanged([this](std::size_t count) {
        QMetaObject::invokeMethod(
            this,
            [this, count] {
                m_pendingSyncCount = static_cast<int>(count);
                emit pendingSyncCountChanged();
            },
            Qt::QueuedConnection);
    });
    m_vendingEngineService.setOnOnlineChanged([this](bool online) {
        QMetaObject::invokeMethod(
            this,
            [this, online] {
                m_online = online;
                emit onlineChanged();
            },
            Qt::QueuedConnection);
    });
}

VendingController::~VendingController() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingController", "~VendingController()");
}

QString VendingController::stateName() const {
    return m_stateName;
}

bool VendingController::dispensing() const {
    return m_dispensing;
}

int VendingController::dispenseProgress() const {
    return m_dispenseProgress;
}

bool VendingController::online() const {
    return m_online;
}

int VendingController::pendingSyncCount() const {
    return m_pendingSyncCount;
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
