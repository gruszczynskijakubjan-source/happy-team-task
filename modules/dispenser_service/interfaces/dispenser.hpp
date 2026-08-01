#pragma once

#include <functional>
#include <string>

namespace vending::dispenser_service {

class IDispenser {
public:
    virtual ~IDispenser() = default;

    virtual void dispense(std::string productId) = 0;

    std::function<void(int)> onProgress;
    std::function<void(bool)> onDispenseResult;
};

}  // namespace vending::dispenser_service
