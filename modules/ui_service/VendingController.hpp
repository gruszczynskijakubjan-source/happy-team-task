#pragma once

#include <QObject>
#include <QString>

#include "cloud_service/impl/cloud_service.hpp"
#include "dispenser_service/interfaces/dispenser.hpp"
#include "rfid_service/interfaces/card_reader.hpp"
#include "shared_helper/logging/logger.hpp"
#include "vending_engine_service/impl/state_machine.hpp"
#include "vending_engine_service/interfaces/transaction_service.hpp"

namespace vending::ui_service {

// The only QObject that talks to the other modules. Exposes state as
// Q_PROPERTYs for QML bindings and forwards UI-originated events
// (simulateCardTap, selectProduct) down into vending_engine_service's
// TransactionService. See DECISIONS.md, "Model wątkowości" — this class
// must not block the GUI thread; engine/db/network work happens off it.
//
// Dependencies (TransactionService, VendingStateMachine, CloudService, and
// everything they're built on — real or mock) are owned and wired by the
// application's composition root (see root main.cpp), not by this class;
// VendingController only holds references into that graph.
//
// dispenser/cardReader are wired here directly (in addition to being wired
// into TransactionService) purely so the GUI can reflect live dispense
// progress and trigger a simulated tap — both dummy implementations call
// back from a non-Qt thread, so those callbacks marshal back onto this
// object's thread via Qt::QueuedConnection before touching any state.
class VendingController final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString stateName READ stateName NOTIFY stateChanged)
    Q_PROPERTY(bool dispensing READ dispensing NOTIFY dispensingChanged)
    Q_PROPERTY(int dispenseProgress READ dispenseProgress NOTIFY dispenseProgressChanged)
    Q_PROPERTY(bool online READ online NOTIFY onlineChanged)
    Q_PROPERTY(int pendingSyncCount READ pendingSyncCount NOTIFY pendingSyncCountChanged)

public:
    VendingController(vending_engine_service::TransactionService& transactionService,
                       vending_engine_service::VendingStateMachine& fsm,
                       cloud_service::CloudService& cloudService, dispenser_service::IDispenser& dispenser,
                       rfid_service::ICardReader& cardReader, shared_helper::logging::ILogger& logger,
                       QObject* parent = nullptr);
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
    vending_engine_service::TransactionService& m_transactionService;
    vending_engine_service::VendingStateMachine& m_fsm;
    cloud_service::CloudService& m_cloudService;
    dispenser_service::IDispenser& m_dispenser;
    rfid_service::ICardReader& m_cardReader;
    shared_helper::logging::ILogger& m_logger;

    bool m_dispensing = false;
    int m_dispenseProgress = 0;
};

}  // namespace vending::ui_service
