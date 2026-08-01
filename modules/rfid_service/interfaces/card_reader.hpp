#pragma once

#include <functional>
#include <string>

namespace vending::rfid_service {

class ICardReader {
public:
    virtual ~ICardReader() = default;

    // Simulates a physical card tap: implementations mint a card id
    // (however they see fit) and invoke onTap with it. Triggered by the UI
    // in the absence of real RFID hardware.
    virtual void simulateTap() = 0;

    std::function<void(std::string)> onTap;
};

}  // namespace vending::rfid_service
