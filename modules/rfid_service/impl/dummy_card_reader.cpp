#include "rfid_service/impl/dummy_card_reader.hpp"

namespace vending::rfid_service {

DummyCardReader::DummyCardReader(shared_helper::UuidGenerator& uuidGen, shared_helper::logging::ILogger& logger)
    : m_uuidGen(uuidGen), m_logger(logger) {}

void DummyCardReader::simulateTap() {
    std::string cardId = m_uuidGen.generate();
    m_logger.log(shared_helper::logging::LogLevel::Trace, "DummyCardReader", "simulateTap() -> " + cardId);
    if (onTap) {
        onTap(cardId);
    }
}

}  // namespace vending::rfid_service
