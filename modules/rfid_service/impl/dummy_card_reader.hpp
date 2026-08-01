#pragma once

#include "rfid_service/interfaces/card_reader.hpp"
#include "shared_helper/interfaces/uuid_generator.hpp"
#include "shared_helper/logging/logger.hpp"

namespace vending::rfid_service {

// No real hardware driver exists yet. simulateTap() mints a card id via
// shared_helper::UuidGenerator and fires onTap with it, standing in for an
// actual card being tapped against the reader.
class DummyCardReader final : public ICardReader {
public:
    DummyCardReader(shared_helper::UuidGenerator& uuidGen, shared_helper::logging::ILogger& logger);

    void simulateTap() override;

private:
    shared_helper::UuidGenerator& m_uuidGen;
    shared_helper::logging::ILogger& m_logger;
};

}  // namespace vending::rfid_service
