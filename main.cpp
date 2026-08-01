// Composition root: owns every concrete implementation (real or mock) and
// wires them into the object graph consumed by vending_engine_service and
// ui_service. Qt/QML is just one leaf of that graph, started last.
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QUrl>

#include "VendingController.hpp"
#include "cloud_service/impl/cloud_service.hpp"
#include "cloud_service/impl/rest_transport.hpp"
#include "cloud_service/impl/sqlite_outbox_storage.hpp"
#include "dispenser_service/impl/dummy_dispenser.hpp"
#include "rfid_service/impl/dummy_card_reader.hpp"
#include "shared_helper/impl/random_uuid_generator.hpp"
#include "shared_helper/impl/system_clock.hpp"
#include "shared_helper/logging/console_logger.hpp"
#include "shared_helper/persistence/json_machine_state_store.hpp"
#include "shared_helper/sql/sqlite_sql_adapter.hpp"
#include "vending_engine_service/impl/vending_engine_service.hpp"

int main(int argc, char* argv[]) {
    using namespace vending;

    QGuiApplication app(argc, argv);

    // TODO: all these objects shall be moved to proper startups/factory
    shared_helper::logging::ConsoleLogger logger;
    shared_helper::SystemClock clock;
    shared_helper::RandomUuidGenerator uuidGen(logger);
    shared_helper::sql::SqliteSqlAdapter sqlAdapter("vending.db", logger);
    shared_helper::persistence::JsonMachineStateStore stateStore("machine_state.json", logger);

    cloud_service::SqliteOutboxStorage outboxStorage(sqlAdapter, logger);
    cloud_service::RestTransport restTransport("http://localhost:8080", logger);
    cloud_service::CloudService cloudService(outboxStorage, restTransport, logger);

    // No real hardware driver exists yet — dummy stand-ins simulate a
    // dispense cycle and a card tap, both triggerable from the UI.
    dispenser_service::DummyDispenser dispenser(clock, logger);

    vending_engine_service::VendingEngineService vendingEngineService(stateStore
        , cloudService
        , dispenser
        , uuidGen
        , clock
        , logger
    );
    vendingEngineService.start();

    // DummyCardReader calls straight into vendingEngineService::onCardTapped()
    // on a simulated tap, so it must be constructed after it.
    rfid_service::DummyCardReader cardReader(uuidGen, vendingEngineService, logger);

    ui_service::VendingController controller(vendingEngineService, cloudService, cardReader, logger);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", &controller);
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreated, &app,
        [](QObject* obj, const QUrl&) {
            if (!obj) {
                QCoreApplication::exit(-1);
            }
        },
        Qt::QueuedConnection);
    engine.load(QUrl(QStringLiteral("qrc:/VendingApp/qml/Main.qml")));

    return app.exec();
}
