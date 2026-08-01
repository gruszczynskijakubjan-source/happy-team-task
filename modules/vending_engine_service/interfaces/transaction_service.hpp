#pragma once

#include <string>

#include "cloud_service/interfaces/cloud_service.hpp"
#include "dispenser_service/interfaces/dispenser.hpp"
#include "rfid_service/interfaces/card_reader.hpp"
#include "shared_helper/interfaces/uuid_generator.hpp"
#include "shared_helper/logging/logger.hpp"
#include "shared_helper/persistence/machine_state_store.hpp"
#include "vending_engine_service/impl/state_machine.hpp"

namespace vending::vending_engine_service {

// vending_engine's public surface: the only component that knows about
// the state machine and every dependency-module's interface at once.
// Wires FSM transitions to shared_helper's MachineStateStore
// (write-before-dispense, so a crash mid-dispense can be recovered on
// restart) and to dispenser_service/rfid_service. See DECISIONS.md,
// Dynamic view.
//
// Once a transaction reaches a terminal state (Completed/Failed),
// TransactionService hands it to cloud_service via ICloudService::send() —
// vending_engine does not track sync status itself; that's cloud_service's
// own concern from then on (see cloud_service/impl/cloud_service.hpp).
class TransactionService {
public:
    TransactionService(VendingStateMachine& fsm, shared_helper::persistence::IMachineStateStore& stateStore,
                        cloud_service::ICloudService& cloudService, dispenser_service::IDispenser& dispenser,
                        rfid_service::ICardReader& cardReader, shared_helper::UuidGenerator& uuidGen,
                        shared_helper::logging::ILogger& logger);

    // Wires m_cardReader.onTap(...) to m_fsm.onCardTapped(...), etc.
    void start();

    void onCardTapped(const std::string& cardId);
    void onProductSelected(const std::string& productId);
    void onDispenseResult(bool ok);

private:
    VendingStateMachine& m_fsm;
    shared_helper::persistence::IMachineStateStore& m_stateStore;
    cloud_service::ICloudService& m_cloudService;
    dispenser_service::IDispenser& m_dispenser;
    rfid_service::ICardReader& m_cardReader;
    shared_helper::UuidGenerator& m_uuidGen;
    shared_helper::logging::ILogger& m_logger;

    std::string m_currentTransactionId;
};

}  // namespace vending::vending_engine_service
