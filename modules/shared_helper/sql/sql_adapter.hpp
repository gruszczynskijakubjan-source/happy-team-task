#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace vending::shared_helper::sql {

using Value = std::variant<std::nullptr_t, std::int64_t, double, std::string>;

using Row = std::vector<Value>;

class ISqlAdapter {
public:
    virtual ~ISqlAdapter() = default;

    virtual std::size_t execute(const std::string& sql, const std::vector<Value>& params = {}) = 0;
    virtual std::vector<Row> query(const std::string& sql, const std::vector<Value>& params = {}) = 0;
};

}  // namespace vending::shared_helper::sql
