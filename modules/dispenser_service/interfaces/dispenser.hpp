#pragma once

#include <functional>
#include <string>

namespace vending::dispenser_service {

class IDispenser {
public:
    virtual ~IDispenser() = default;

    // Registers the progress/result callbacks once, up front; dispense()
    // calls fire whichever callbacks are currently registered rather than
    // taking them per-call.
    virtual void setOnProgress(std::function<void(int)> onProgress) = 0;
    virtual void setOnResult(std::function<void(bool)> onResult) = 0;

    // Triggers the physical dispense for productId. Progress/result are
    // reported asynchronously via the callbacks registered above,
    // mirroring the real hardware's fire-and-forget dispense cycle.
    virtual void dispense(std::string productId) = 0;
};

}  // namespace vending::dispenser_service
