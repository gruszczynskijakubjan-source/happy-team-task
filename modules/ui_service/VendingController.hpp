#pragma once

#include <QObject>
#include <QString>

#include "cloud_service/impl/cloud_service.hpp"
#include "rfid_service/interfaces/card_reader.hpp"
#include "shared_helper/logging/logger.hpp"
#include "vending_engine_service/interfaces/vending_engine_service.hpp"

namespace vending::ui_service {

class VendingController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString stateName READ stateName NOTIFY stateChanged)
    Q_PROPERTY(bool dispensing READ dispensing NOTIFY dispensingChanged)
    Q_PROPERTY(int dispenseProgress READ dispenseProgress NOTIFY dispenseProgressChanged)
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(int pendingSyncCount READ pendingSyncCount NOTIFY pendingSyncCountChanged)

 public:
    VendingController(vending_engine_service::IVendingEngineService& vendingEngineService
        , cloud_service::CloudService& cloudService
        , rfid_service::ICardReader& cardReader
        , shared_helper::logging::ILogger& logger
        , QObject* parent = nullptr
    );
    ~VendingController() override;

    QString stateName() const;
    bool dispensing() const;
    int dispenseProgress() const;
    bool online() const;
    int pendingSyncCount() const;

 public slots:
    // Bound to the "Symuluj przyłożenie karty" button in QML.
    void simulateCardTap();
    void selectProduct(const QString& productId);

 signals:
    void stateChanged();
    void dispensingChanged();
    void dispenseProgressChanged();
    void onlineChanged();
    void pendingSyncCountChanged();

 private:
    vending_engine_service::IVendingEngineService& m_vendingEngineService;
    cloud_service::CloudService& m_cloudService;
    rfid_service::ICardReader& m_cardReader;
    shared_helper::logging::ILogger& m_logger;

    bool m_dispensing = false;
    int m_dispenseProgress = 0;
    QString m_stateName;
};

}  // namespace vending::ui_service
