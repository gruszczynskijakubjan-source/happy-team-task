#include "shared_helper/impl/random_uuid_generator.hpp"

namespace vending::shared_helper {

RandomUuidGenerator::RandomUuidGenerator(logging::ILogger& logger) : m_logger(logger) {
    m_logger.log(logging::LogLevel::Trace, "RandomUuidGenerator", "RandomUuidGenerator()");
}

std::string RandomUuidGenerator::generate() {
    m_logger.log(logging::LogLevel::Trace, "RandomUuidGenerator", "generate()");
    // TODO: implement
    return {};
}

}  // namespace vending::shared_helper
