#pragma once

#include <chrono>
#include <optional>
#include <string>

namespace vending::shared_helper {

enum class TxStatus {
    Pending,
    Selected,
    Dispensing,
    Completed,
    Failed,
    UnknownNeedsReconciliation,
};

struct TransactionRecord {
    std::string id;  // UUID, minted by vending_engine_service via shared_helper::UuidGenerator
    std::string cardId;
    std::optional<std::string> productId;
    TxStatus status = TxStatus::Pending;
    std::chrono::system_clock::time_point createdAt;
    std::chrono::system_clock::time_point updatedAt;
    std::optional<std::chrono::system_clock::time_point> syncedAt;
};

}  // namespace vending::shared_helper
