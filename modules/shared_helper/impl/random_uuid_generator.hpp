#pragma once

#include "shared_helper/interfaces/uuid_generator.hpp"
#include "shared_helper/logging/logger.hpp"

namespace vending::shared_helper {

class RandomUuidGenerator final : public UuidGenerator {
public:
    explicit RandomUuidGenerator(logging::ILogger& logger);

    std::string generate() override;

private:
    logging::ILogger& m_logger;
};

}  // namespace vending::shared_helper
