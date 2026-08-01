#pragma once

#include "rfid_service/interfaces/card_reader.hpp"
#include "shared_helper/interfaces/uuid_generator.hpp"
#include "shared_helper/logging/logger.hpp"
#include "vending_engine_service/interfaces/vending_engine_service.hpp"

namespace vending::rfid_service {

// No real hardware driver exists yet. simulateTap() mints a card id via
// shared_helper::UuidGenerator and calls the injected vending engine's
// onCardTapped() directly, standing in for an actual card being tapped
// against the reader.
class DummyCardReader final : public ICardReader {
public:
    DummyCardReader(shared_helper::UuidGenerator& uuidGen,
                     vending_engine_service::IVendingEngineService& vendingEngineService,
                     shared_helper::logging::ILogger& logger);

    void simulateTap() override;

private:
    shared_helper::UuidGenerator& m_uuidGen;
    vending_engine_service::IVendingEngineService& m_vendingEngineService;
    shared_helper::logging::ILogger& m_logger;
};

}  // namespace vending::rfid_service
