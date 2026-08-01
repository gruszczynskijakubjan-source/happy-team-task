#pragma once

#include <gmock/gmock.h>

#include "rfid_service/interfaces/card_reader.hpp"

namespace vending::rfid_service::mocks {

class MockCardReader : public ICardReader {
public:
    MOCK_METHOD(void, simulateTap, (), (override));
};

}  // namespace vending::rfid_service::mocks
