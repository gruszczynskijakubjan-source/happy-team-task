#include "rfid_service/impl/dummy_card_reader.hpp"

namespace vending::rfid_service {

DummyCardReader::DummyCardReader(shared_helper::UuidGenerator& uuidGen,
                                  vending_engine_service::IVendingEngineService& vendingEngineService,
                                  shared_helper::logging::ILogger& logger)
    : m_uuidGen(uuidGen), m_vendingEngineService(vendingEngineService), m_logger(logger) {}

void DummyCardReader::simulateTap() {
    const std::string cardId = m_uuidGen.generate();
    m_logger.log(shared_helper::logging::LogLevel::Trace, "DummyCardReader", "simulateTap() -> " + cardId);
    m_vendingEngineService.onCardTapped(cardId);
}

}  // namespace vending::rfid_service
