#pragma once

namespace vending::rfid_service {

class ICardReader {
public:
    virtual ~ICardReader() = default;

    virtual void simulateTap() = 0;
};

}  // namespace vending::rfid_service
