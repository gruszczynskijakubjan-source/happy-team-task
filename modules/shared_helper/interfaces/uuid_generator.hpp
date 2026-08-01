#pragma once

#include <string>

namespace vending::shared_helper {

class UuidGenerator {
public:
    virtual ~UuidGenerator() = default;
    virtual std::string generate() = 0;
};

}  // namespace vending::shared_helper
