#pragma once

#include "shared_helper/logging/logger.hpp"

namespace vending::shared_helper::logging {

class ConsoleLogger final : public ILogger {
public:
    void log(LogLevel level, std::string_view component, std::string_view message) override;
};

}  // namespace vending::shared_helper::logging
