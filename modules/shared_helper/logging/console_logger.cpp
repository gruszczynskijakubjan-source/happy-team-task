#include "shared_helper/logging/console_logger.hpp"

#include <iostream>

namespace vending::shared_helper::logging {

namespace {

const char* levelName(LogLevel level) {
    switch (level) {
        case LogLevel::Trace:
            return "TRACE";
        case LogLevel::Debug:
            return "DEBUG";
        case LogLevel::Info:
            return "INFO";
        case LogLevel::Warn:
            return "WARN";
        case LogLevel::Error:
            return "ERROR";
    }
    return "UNKNOWN";
}

}  // namespace

void ConsoleLogger::log(LogLevel level, std::string_view component, std::string_view message) {
    std::cout << "[" << levelName(level) << "] " << component << ": " << message << "\n";
}

}  // namespace vending::shared_helper::logging
