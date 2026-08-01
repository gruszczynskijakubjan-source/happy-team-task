#pragma once

#include <algorithm>
#include <map>

#include "shared_helper/interfaces/clock.hpp"

namespace vending::shared_helper::mocks {

class FakeClock final : public IClock {
public:
    std::chrono::system_clock::time_point now() const override { return m_now; }

    TimerHandle scheduleOnce(std::chrono::milliseconds delay, std::function<void()> cb) override {
        const TimerHandle handle = m_nextHandle++;
        m_timers[handle] = {m_now + delay, std::move(cb)};
        return handle;
    }

    void cancel(TimerHandle handle) override { m_timers.erase(handle); }

    void cancelAll() override { m_timers.clear(); }

    void advance(std::chrono::milliseconds delta) {
        m_now += delta;
        for (;;) {
            auto due = std::find_if(m_timers.begin(), m_timers.end(),
                                     [this](const auto& entry) { return entry.second.deadline <= m_now; });
            if (due == m_timers.end()) {
                break;
            }
            auto cb = std::move(due->second.cb);
            m_timers.erase(due);
            cb();
        }
    }

private:
    struct Timer {
        std::chrono::system_clock::time_point deadline;
        std::function<void()> cb;
    };

    std::chrono::system_clock::time_point m_now{};
    std::map<TimerHandle, Timer> m_timers;
    TimerHandle m_nextHandle = InvalidTimerHandle + 1;
};

}  // namespace vending::shared_helper::mocks
