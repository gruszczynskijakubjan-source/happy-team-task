#include "vending_engine_service/impl/vending_engine_service.hpp"

namespace vending::vending_engine_service {

VendingEngineService::VendingEngineService(shared_helper::persistence::IMachineStateStore& stateStore
    , cloud_service::ICloudService& cloudService
    , dispenser_service::IDispenser& dispenser
    , shared_helper::UuidGenerator& uuidGen
    , shared_helper::logging::ILogger& logger)
        : m_stateStore(stateStore)
        , m_cloudService(cloudService)
        , m_dispenser(dispenser)
        , m_uuidGen(uuidGen)
        , m_logger(logger) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "VendingEngineService()");
    // TODO: implement

    m_dispenser.setOnProgress([this](int progress) {
        if (m_onDispenseProgress) {
            m_onDispenseProgress(progress);
        }
    });
    m_dispenser.setOnResult([this](bool ok) {
        onDispenseResult(ok);
    });
}

void VendingEngineService::start() {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "start()");
    // TODO: implement
}

void VendingEngineService::onCardTapped(const std::string& cardId) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "onCardTapped(" + cardId + ")");
    // TODO: implement
}

void VendingEngineService::onProductSelected(const std::string& productId) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService",
                 "onProductSelected(" + productId + ")");
    // TODO: implement
    m_dispenser.dispense(productId);
}

void VendingEngineService::onDispenseResult(bool ok) {
    m_logger.log(shared_helper::logging::LogLevel::Trace, "VendingEngineService", "onDispenseResult()");
    if (m_onDispenseFinished) {
        m_onDispenseFinished(ok);
    }
}

void VendingEngineService::setOnDispenseProgress(std::function<void(int)> onDispenseProgress) {
    m_onDispenseProgress = std::move(onDispenseProgress);
}

void VendingEngineService::setOnDispenseFinished(std::function<void(bool)> onDispenseFinished) {
    m_onDispenseFinished = std::move(onDispenseFinished);
}

}  // namespace vending::vending_engine_service
