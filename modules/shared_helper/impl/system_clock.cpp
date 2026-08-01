#include "shared_helper/impl/system_clock.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace vending::shared_helper {

struct SystemClock::Impl {
    std::vector<std::jthread> timers;
};

SystemClock::SystemClock() : m_impl(std::make_unique<Impl>()) {}

SystemClock::~SystemClock() {
    cancelAll();
}

std::chrono::system_clock::time_point SystemClock::now() const {
    return std::chrono::system_clock::now();
}

void SystemClock::scheduleOnce(std::chrono::milliseconds delay, std::function<void()> cb) {
    m_impl->timers.emplace_back([delay, cb = std::move(cb)](std::stop_token stopToken) {
        std::condition_variable_any cv;
        std::mutex mtx;
        std::unique_lock lock(mtx);
        if (!cv.wait_for(lock, stopToken, delay, [] { return false; })) {
            cb();
        }
    });
}

void SystemClock::cancelAll() {
    m_impl->timers.clear();
}

}  // namespace vending::shared_helper
