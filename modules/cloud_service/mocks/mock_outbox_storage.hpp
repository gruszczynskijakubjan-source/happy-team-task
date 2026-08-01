#pragma once

#include <gmock/gmock.h>

#include "cloud_service/interfaces/outbox_storage.hpp"

namespace vending::cloud_service::mocks {

class MockOutboxStorage : public IOutboxStorage {
public:
    MOCK_METHOD(bool, store, (const shared_helper::TransactionRecord& tx), (override));
    MOCK_METHOD(std::optional<shared_helper::TransactionRecord>, getOldestUnsynced, (), (override));
    MOCK_METHOD(std::size_t, countUnsynced, (), (override));
    MOCK_METHOD(void, markSynced, (const std::string& id), (override));
};

}  // namespace vending::cloud_service::mocks
