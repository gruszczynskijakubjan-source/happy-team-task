#pragma once

#include <string>

#include "cloud_service/interfaces/cloud_service.hpp"
#include "dispenser_service/interfaces/dispenser.hpp"
#include "shared_helper/interfaces/uuid_generator.hpp"
#include "shared_helper/logging/logger.hpp"
#include "shared_helper/persistence/machine_state_store.hpp"
#include "vending_engine_service/interfaces/vending_engine_service.hpp"

namespace vending::vending_engine_service {

class VendingEngineService final : public IVendingEngineService {
public:
    VendingEngineService(shared_helper::persistence::IMachineStateStore& stateStore
        , cloud_service::ICloudService& cloudService
        , dispenser_service::IDispenser& dispenser
        , shared_helper::UuidGenerator& uuidGen
        , shared_helper::logging::ILogger& logger
    );

    void start() override;

    void onCardTapped(const std::string& cardId) override;
    void onProductSelected(const std::string& productId) override;
    void onDispenseResult(bool ok) override;

    void setOnDispenseProgress(std::function<void(int)> onDispenseProgress) override;
    void setOnDispenseFinished(std::function<void(bool)> onDispenseFinished) override;

private:
    shared_helper::persistence::IMachineStateStore& m_stateStore;
    cloud_service::ICloudService& m_cloudService;
    dispenser_service::IDispenser& m_dispenser;
    shared_helper::UuidGenerator& m_uuidGen;
    shared_helper::logging::ILogger& m_logger;

    std::string m_currentTransactionId;
    std::function<void(int)> m_onDispenseProgress;
    std::function<void(bool)> m_onDispenseFinished;
};

}  // namespace vending::vending_engine_service
