#pragma once

#include <cstddef>
#include <functional>
#include <string>

namespace vending::vending_engine_service {

// Dispense cycle:
//   Idle -> CardRead -> ProductSelected -> Dispensing -> Completed
//                                                      \> Failed
// CardRead has a selection timeout (see VendingEngineService) that returns
// to Idle if no product is picked in time.
enum class State {
    Idle,
    CardRead,
    ProductSelected,
    Dispensing,
    Completed,
    Failed,
};

class IVendingEngineService {
 public:
    virtual ~IVendingEngineService() = default;

    virtual void start() = 0;

    virtual void onCardTapped(const std::string& cardId) = 0;
    virtual void onProductSelected(const std::string& productId) = 0;
    virtual void onDispenseResult(bool ok) = 0;

    virtual State state() const = 0;

    // Registered once, up front, by whoever wants to observe a dispense in
    // flight (e.g. VendingController for the UI's progress bar). Forwarded
    // from IDispenser's own callbacks.
    virtual void setOnDispenseProgress(std::function<void(int)> onDispenseProgress) = 0;
    virtual void setOnDispenseFinished(std::function<void(bool)> onDispenseFinished) = 0;

    // Fired whenever the FSM transitions; lets UI code react without
    // polling state().
    virtual void setOnStateChanged(std::function<void(State)> onStateChanged) = 0;

    // Forwarded from ICloudService's own callbacks, so UI code only ever
    // has to depend on this interface (see VendingEngineService, which
    // subscribes to cloud_service and re-publishes here).
    virtual void setOnPendingSyncCountChanged(std::function<void(std::size_t)> onPendingSyncCountChanged) = 0;
    virtual void setOnOnlineChanged(std::function<void(bool)> onOnlineChanged) = 0;
};

}  // namespace vending::vending_engine_service
