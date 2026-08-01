#pragma once

#include <chrono>
#include <cstdint>
#include <functional>

namespace vending::shared_helper {

// Opaque handle to a single scheduleOnce() call, for cancel(). 0 is never
// issued by a real implementation, so it doubles as a null/"nothing
// scheduled" sentinel for callers that only ever track one timer.
using TimerHandle = std::uint64_t;
inline constexpr TimerHandle InvalidTimerHandle = 0;

class IClock {
 public:
    virtual ~IClock() = default;
    virtual std::chrono::system_clock::time_point now() const = 0;
    virtual TimerHandle scheduleOnce(std::chrono::milliseconds delay, std::function<void()> cb) = 0;
    // No-op if handle is stale (already fired) or InvalidTimerHandle.
    virtual void cancel(TimerHandle handle) = 0;
    virtual void cancelAll() = 0;
};

}  // namespace vending::shared_helper
