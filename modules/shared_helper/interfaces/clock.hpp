#pragma once

#include <chrono>
#include <functional>

namespace vending::shared_helper {

class IClock {
 public:
    virtual ~IClock() = default;
    virtual std::chrono::system_clock::time_point now() const = 0;
    virtual void scheduleOnce(std::chrono::milliseconds delay, std::function<void()> cb) = 0;
    virtual void cancelAll() = 0;
};

}  // namespace vending::shared_helper
