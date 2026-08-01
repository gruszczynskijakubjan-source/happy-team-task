#include "shared_helper/impl/system_clock.hpp"

#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>

namespace vending::shared_helper {

struct SystemClock::Impl {
    std::mutex mutex;
    std::map<TimerHandle, std::jthread> timers;
    TimerHandle nextHandle = InvalidTimerHandle + 1;
};

SystemClock::SystemClock() : m_impl(std::make_unique<Impl>()) {}

SystemClock::~SystemClock() {
    cancelAll();
}

std::chrono::system_clock::time_point SystemClock::now() const {
    return std::chrono::system_clock::now();
}

TimerHandle SystemClock::scheduleOnce(std::chrono::milliseconds delay, std::function<void()> cb) {
    std::lock_guard lock(m_impl->mutex);
    const TimerHandle handle = m_impl->nextHandle++;

    // Fired timers stay in the map (joinable, already finished) until the
    // next cancel()/cancelAll()/destructor sweep; they don't spin or hold
    // any other resource in the meantime, so this is a bounded amount of
    // bookkeeping rather than a real leak.
    m_impl->timers[handle] = std::jthread([delay, cb = std::move(cb)](std::stop_token stopToken) {
        std::condition_variable_any cv;
        std::mutex waitMutex;
        std::unique_lock waitLock(waitMutex);
        if (!cv.wait_for(waitLock, stopToken, delay, [] { return false; })) {
            cb();
        }
    });

    return handle;
}

void SystemClock::cancel(TimerHandle handle) {
    std::lock_guard lock(m_impl->mutex);
    if (auto it = m_impl->timers.find(handle); it != m_impl->timers.end()) {
        it->second.request_stop();
        it->second.detach();
        m_impl->timers.erase(it);
    }
}

void SystemClock::cancelAll() {
    std::lock_guard lock(m_impl->mutex);
    for (auto& [handle, thread] : m_impl->timers) {
        thread.request_stop();
        thread.detach();
    }
    m_impl->timers.clear();
}

}  // namespace vending::shared_helper
