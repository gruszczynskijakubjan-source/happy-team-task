#include "shared_helper/impl/random_uuid_generator.hpp"

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace vending::shared_helper {

RandomUuidGenerator::RandomUuidGenerator(logging::ILogger& logger)
    : m_logger(logger) {
    m_logger.log(logging::LogLevel::Trace, "RandomUuidGenerator", "RandomUuidGenerator()");
}

std::string RandomUuidGenerator::generate() {
    m_logger.log(logging::LogLevel::Trace, "RandomUuidGenerator", "generate()");

    static thread_local boost::uuids::random_generator generator;
    return boost::uuids::to_string(generator());
}

}  // namespace vending::shared_helper
