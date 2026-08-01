#pragma once

#include <functional>
#include <string>

namespace vending::vending_engine_service {

class IVendingEngineService {
 public:
    virtual ~IVendingEngineService() = default;

    virtual void start() = 0;

    virtual void onCardTapped(const std::string& cardId) = 0;
    virtual void onProductSelected(const std::string& productId) = 0;
    virtual void onDispenseResult(bool ok) = 0;

    // Registered once, up front, by whoever wants to observe a dispense in
    // flight (e.g. VendingController for the UI's progress bar). Forwarded
    // from IDispenser's own callbacks.
    virtual void setOnDispenseProgress(std::function<void(int)> onDispenseProgress) = 0;
    virtual void setOnDispenseFinished(std::function<void(bool)> onDispenseFinished) = 0;
};

}  // namespace vending::vending_engine_service
