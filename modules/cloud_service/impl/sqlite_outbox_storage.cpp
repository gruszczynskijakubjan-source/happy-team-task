#include "cloud_service/impl/sqlite_outbox_storage.hpp"

#include <chrono>

namespace vending::cloud_service {

namespace {

using shared_helper::sql::Value;

std::string toString(shared_helper::TxStatus status) {
    switch (status) {
        case shared_helper::TxStatus::Pending:
            return "Pending";
        case shared_helper::TxStatus::Selected:
            return "Selected";
        case shared_helper::TxStatus::Dispensing:
            return "Dispensing";
        case shared_helper::TxStatus::Completed:
            return "Completed";
        case shared_helper::TxStatus::Failed:
            return "Failed";
        case shared_helper::TxStatus::UnknownNeedsReconciliation:
            return "UnknownNeedsReconciliation";
    }
    return "UnknownNeedsReconciliation";
}

shared_helper::TxStatus toTxStatus(const std::string& s) {
    if (s == "Pending") return shared_helper::TxStatus::Pending;
    if (s == "Selected") return shared_helper::TxStatus::Selected;
    if (s == "Dispensing") return shared_helper::TxStatus::Dispensing;
    if (s == "Completed") return shared_helper::TxStatus::Completed;
    if (s == "Failed") return shared_helper::TxStatus::Failed;
    return shared_helper::TxStatus::UnknownNeedsReconciliation;
}

std::int64_t toMillis(std::chrono::system_clock::time_point tp) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();
}

std::chrono::system_clock::time_point fromMillis(std::int64_t millis) {
    return std::chrono::system_clock::time_point(std::chrono::milliseconds(millis));
}

Value optionalToValue(const std::optional<std::string>& v) {
    return v ? Value(*v) : Value(nullptr);
}

Value optionalTimeToValue(const std::optional<std::chrono::system_clock::time_point>& v) {
    return v ? Value(toMillis(*v)) : Value(nullptr);
}

std::optional<std::string> asOptionalString(const Value& v) {
    if (std::holds_alternative<std::nullptr_t>(v)) {
        return std::nullopt;
    }
    return std::get<std::string>(v);
}

std::optional<std::chrono::system_clock::time_point> asOptionalTime(const Value& v) {
    if (std::holds_alternative<std::nullptr_t>(v)) {
        return std::nullopt;
    }
    return fromMillis(std::get<std::int64_t>(v));
}

}  // namespace

SqliteOutboxStorage::SqliteOutboxStorage(shared_helper::sql::ISqlAdapter& sqlAdapter
    , shared_helper::logging::ILogger& logger)
        : m_sqlAdapter(sqlAdapter)
        , m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "SqliteOutboxStorage()");
    ensureSchema();
}

void SqliteOutboxStorage::ensureSchema() {
    m_sqlAdapter.execute(
        "CREATE TABLE IF NOT EXISTS outbox ("
        "  id TEXT PRIMARY KEY,"
        "  card_id TEXT NOT NULL,"
        "  product_id TEXT,"
        "  status TEXT NOT NULL,"
        "  created_at INTEGER NOT NULL,"
        "  updated_at INTEGER NOT NULL,"
        "  synced_at INTEGER"
        ")");
    m_sqlAdapter.execute(
        "CREATE INDEX IF NOT EXISTS idx_outbox_unsynced ON outbox (created_at) WHERE synced_at IS NULL");
}

bool SqliteOutboxStorage::store(const shared_helper::TransactionRecord& tx) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "store(" + tx.id + ")");

    // INSERT OR IGNORE: a re-store() of a transaction id already present is
    // a silent no-op at the SQL level; sqlite3_changes() tells us whether a
    // row was actually inserted so we can log the duplicate explicitly.
    const std::size_t changed = m_sqlAdapter.execute(
        "INSERT OR IGNORE INTO outbox (id, card_id, product_id, status, created_at, updated_at, synced_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?)",
        {Value(tx.id), Value(tx.cardId), optionalToValue(tx.productId), Value(toString(tx.status)),
         Value(toMillis(tx.createdAt)), Value(toMillis(tx.updatedAt)), optionalTimeToValue(tx.syncedAt)});

    if (changed == 0) {
        m_logger.log(shared_helper::logging::LogLevel::Warn, "SqliteOutboxStorage",
                     "store(" + tx.id + ") ignored, id already present");
        return false;
    }
    return true;
}

std::optional<shared_helper::TransactionRecord> SqliteOutboxStorage::getOldestUnsynced() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "getOldestUnsynced()");

    const auto rows = m_sqlAdapter.query(
        "SELECT id, card_id, product_id, status, created_at, updated_at, synced_at "
        "FROM outbox WHERE synced_at IS NULL ORDER BY created_at ASC LIMIT 1");

    if (rows.empty()) {
        return std::nullopt;
    }

    const auto& row = rows.front();
    shared_helper::TransactionRecord tx;
    tx.id = std::get<std::string>(row[0]);
    tx.cardId = std::get<std::string>(row[1]);
    tx.productId = asOptionalString(row[2]);
    tx.status = toTxStatus(std::get<std::string>(row[3]));
    tx.createdAt = fromMillis(std::get<std::int64_t>(row[4]));
    tx.updatedAt = fromMillis(std::get<std::int64_t>(row[5]));
    tx.syncedAt = asOptionalTime(row[6]);
    return tx;
}

std::size_t SqliteOutboxStorage::countUnsynced() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "countUnsynced()");

    const auto rows = m_sqlAdapter.query("SELECT COUNT(*) FROM outbox WHERE synced_at IS NULL");
    if (rows.empty()) {
        return 0;
    }
    return static_cast<std::size_t>(std::get<std::int64_t>(rows.front()[0]));
}

void SqliteOutboxStorage::markSynced(const std::string& id) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "SqliteOutboxStorage", "markSynced(" + id + ")");

    const auto now = std::chrono::system_clock::now();
    m_sqlAdapter.execute("UPDATE outbox SET synced_at = ?, updated_at = ? WHERE id = ?",
                         {Value(toMillis(now)), Value(toMillis(now)), Value(id)});
}

}  // namespace vending::cloud_service
