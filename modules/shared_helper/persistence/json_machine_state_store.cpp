#include "shared_helper/persistence/json_machine_state_store.hpp"

#include <chrono>
#include <cstdio>
#include <fstream>
#include <sstream>

#include <fcntl.h>
#include <unistd.h>

#include <nlohmann/json.hpp>

namespace vending::shared_helper::persistence {

namespace {

using nlohmann::json;

std::string toIso8601(std::chrono::system_clock::time_point tp) {
    return std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count());
}

std::chrono::system_clock::time_point fromIso8601(const std::string& millisSinceEpoch) {
    return std::chrono::system_clock::time_point(
        std::chrono::milliseconds(std::stoll(millisSinceEpoch)));
}

std::string toString(TxStatus status) {
    switch (status) {
        case TxStatus::Pending: return "Pending";
        case TxStatus::Selected: return "Selected";
        case TxStatus::Dispensing: return "Dispensing";
        case TxStatus::Completed: return "Completed";
        case TxStatus::Failed: return "Failed";
        case TxStatus::UnknownNeedsReconciliation: return "UnknownNeedsReconciliation";
    }
    return "UnknownNeedsReconciliation";
}

TxStatus toTxStatus(const std::string& s) {
    if (s == "Pending") return TxStatus::Pending;
    if (s == "Selected") return TxStatus::Selected;
    if (s == "Dispensing") return TxStatus::Dispensing;
    if (s == "Completed") return TxStatus::Completed;
    if (s == "Failed") return TxStatus::Failed;
    return TxStatus::UnknownNeedsReconciliation;
}

json toJson(const TransactionRecord& tx) {
    json j;
    j["id"] = tx.id;
    j["cardId"] = tx.cardId;
    j["productId"] = tx.productId.has_value() ? json(*tx.productId) : json(nullptr);
    j["status"] = toString(tx.status);
    j["createdAt"] = toIso8601(tx.createdAt);
    j["updatedAt"] = toIso8601(tx.updatedAt);
    j["syncedAt"] = tx.syncedAt.has_value() ? json(toIso8601(*tx.syncedAt)) : json(nullptr);
    return j;
}

TransactionRecord fromJson(const json& j) {
    TransactionRecord tx;
    tx.id = j.at("id").get<std::string>();
    tx.cardId = j.at("cardId").get<std::string>();
    if (!j.at("productId").is_null()) {
        tx.productId = j.at("productId").get<std::string>();
    }
    tx.status = toTxStatus(j.at("status").get<std::string>());
    tx.createdAt = fromIso8601(j.at("createdAt").get<std::string>());
    tx.updatedAt = fromIso8601(j.at("updatedAt").get<std::string>());
    if (!j.at("syncedAt").is_null()) {
        tx.syncedAt = fromIso8601(j.at("syncedAt").get<std::string>());
    }
    return tx;
}

}  // namespace

JsonMachineStateStore::JsonMachineStateStore(std::string path, logging::ILogger& logger)
    : m_path(std::move(path))
    , m_logger(logger) {
    m_logger.log(logging::LogLevel::Trace, "JsonMachineStateStore", "JsonMachineStateStore()");
}

void JsonMachineStateStore::save(const MachineState& state) {
    m_logger.log(logging::LogLevel::Trace, "JsonMachineStateStore", "save()");
    json j {};
    j["fsmState"] = state.fsmState;
    j["activeTransaction"] =state.activeTransaction.has_value() ? toJson(*state.activeTransaction) : json(nullptr);

    const std::string tmpPath = m_path + ".tmp";
    {
        std::ofstream out(tmpPath, std::ios::trunc);
        out << j.dump(2);
        out.flush();

        if (int fd = ::open(tmpPath.c_str(), O_WRONLY); fd >= 0) {
            ::fsync(fd);
            ::close(fd);
        }
    }

    std::rename(tmpPath.c_str(), m_path.c_str());

    const auto lastSlash = m_path.find_last_of('/');
    const std::string dir = lastSlash == std::string::npos ? "." : m_path.substr(0, lastSlash);
    if (int dirFd = ::open(dir.c_str(), O_RDONLY); dirFd >= 0) {
        ::fsync(dirFd);
        ::close(dirFd);
    }
}

std::optional<MachineState> JsonMachineStateStore::load() {
    m_logger.log(logging::LogLevel::Trace, "JsonMachineStateStore", "load()");
    std::ifstream in(m_path);
    if (!in.is_open()) {
        return std::nullopt;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();

    json j;
    try {
        j = json::parse(buffer.str());
    } catch (const json::parse_error&) {
        return std::nullopt;
    }

    MachineState state;
    state.fsmState = j.value("fsmState", std::string{});
    if (j.contains("activeTransaction") && !j.at("activeTransaction").is_null()) {
        state.activeTransaction = fromJson(j.at("activeTransaction"));
    }
    return state;
}

void JsonMachineStateStore::clear() {
    m_logger.log(logging::LogLevel::Trace, "JsonMachineStateStore", "clear()");
    std::remove(m_path.c_str());
}

}  // namespace vending::shared_helper::persistence
