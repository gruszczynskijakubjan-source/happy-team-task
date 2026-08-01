#pragma once

#include <memory>

#include "shared_helper/interfaces/clock.hpp"

namespace vending::shared_helper {

// Real-time IClock backed by std::chrono::system_clock. scheduleOnce()
// fires the callback from a detached timer thread, so callers that touch
// non-thread-safe state (e.g. Qt objects) must marshal back to their own
// thread themselves.
class SystemClock final : public IClock {
public:
    SystemClock();
    ~SystemClock() override;

    SystemClock(const SystemClock&) = delete;
    SystemClock& operator=(const SystemClock&) = delete;

    std::chrono::system_clock::time_point now() const override;
    TimerHandle scheduleOnce(std::chrono::milliseconds delay, std::function<void()> cb) override;
    void cancel(TimerHandle handle) override;
    void cancelAll() override;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace vending::shared_helper
