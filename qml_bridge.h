// qml_bridge.h - QML Bridge for Mission Planner

#pragma once

#include "mission_planner.h"
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QTimer>

class QMLBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString missionState READ getMissionState NOTIFY missionStateChanged)
    Q_PROPERTY(int currentWaypoint READ getCurrentWaypoint NOTIFY waypointChanged)
    Q_PROPERTY(int totalWaypoints READ getTotalWaypoints NOTIFY waypointsUpdated)
    Q_PROPERTY(double estimatedTime READ getEstimatedTime NOTIFY statisticsUpdated)
    Q_PROPERTY(double totalDistance READ getTotalDistance NOTIFY statisticsUpdated)
    Q_PROPERTY(double perimeterArea READ getPerimeterArea NOTIFY statisticsUpdated)
    Q_PROPERTY(QString executionMode READ getExecutionMode NOTIFY modeChanged)

public:
    explicit QMLBridge(QObject* parent = nullptr);
    ~QMLBridge();

    // QML callable methods
    Q_INVOKABLE bool initializeMission(bool simulate_mode);
    Q_INVOKABLE bool setPerimeter(double lat1, double lon1, double lat2, double lon2,
                                   double lat3, double lon3, double lat4, double lon4);
    Q_INVOKABLE bool generateMission();
    Q_INVOKABLE bool startMission();
    Q_INVOKABLE bool pauseMission();
    Q_INVOKABLE bool resumeMission();
    Q_INVOKABLE bool cancelMission();
    Q_INVOKABLE bool setMode(bool simulate_mode);
    Q_INVOKABLE QVariantList getWaypoints();
    Q_INVOKABLE QVariantList getPerimeterCorners();
    Q_INVOKABLE bool exportMission(const QString& filepath);
    Q_INVOKABLE bool importMission(const QString& filepath);
    Q_INVOKABLE void updateDronePosition(double lat, double lon, double alt);
    Q_INVOKABLE QString getStatusString();
    Q_INVOKABLE double getMissionProgress();

    // Property getters
    QString getMissionState() const;
    int getCurrentWaypoint() const;
    int getTotalWaypoints() const;
    double getEstimatedTime() const;
    double getTotalDistance() const;
    double getPerimeterArea() const;
    QString getExecutionMode() const;

signals:
    void missionStateChanged();
    void waypointChanged();
    void waypointsUpdated();
    void statisticsUpdated();
    void modeChanged();
    void errorOccurred(const QString& error);
    void dronePositionUpdated(double lat, double lon, double alt);
    void missionProgress(double progress);

private slots:
    void onSimulatorTick();

private:
    MissionPlanner* mission_planner;
    QTimer* simulator_timer;
    int simulator_waypoint_index;
    double current_drone_lat, current_drone_lon, current_drone_alt;

    /**
     * Simulate drone movement between waypoints
     */
    void simulateDroneMovement();

    /**
     * Convert mission state to string
     */
    QString stateToString(MissionState state) const;
};

