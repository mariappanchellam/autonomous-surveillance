// main.cpp - Application entry point

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "qml_bridge.h"

int main(int argc, char *argv[]) {
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Create QML bridge
    QMLBridge missionBridge;

    // Make it available to QML
    engine.rootContext()->setContextProperty("missionBridge", &missionBridge);

    // Load main QML file
    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine.load(url);

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
