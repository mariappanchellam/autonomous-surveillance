// qml_bridge.cpp - Implementation

#include "qml_bridge.h"
#include <QDebug>
#include <QVariantMap>
#include <cmath>

QMLBridge::QMLBridge(QObject* parent)
    : QObject(parent), mission_planner(nullptr), simulator_timer(nullptr),
      simulator_waypoint_index(0), current_drone_lat(0), current_drone_lon(0),
      current_drone_alt(0) {

    mission_planner = new MissionPlanner();

    // Setup simulator timer (update every 100ms)
    simulator_timer = new QTimer(this);
    connect(simulator_timer, &QTimer::timeout, this, &QMLBridge::onSimulatorTick);
}

QMLBridge::~QMLBridge() {
    if (simulator_timer) {
        simulator_timer->stop();
        delete simulator_timer;
    }
    if (mission_planner) {
        delete mission_planner;
    }
}

bool QMLBridge::initializeMission(bool simulate_mode) {
    MissionMode mode = simulate_mode ? MissionMode::SIMULATE : MissionMode::REAL;
    bool result = mission_planner->initialize(mode);

    if (result) {
        emit modeChanged();
    } else {
        emit errorOccurred("Failed to initialize mission");
    }

    return result;
}

bool QMLBridge::setPerimeter(double lat1, double lon1, double lat2, double lon2,
                              double lat3, double lon3, double lat4, double lon4) {
    Coordinate corners[4] = {
        {lat1, lon1, DEFAULT_ALTITUDE},
        {lat2, lon2, DEFAULT_ALTITUDE},
        {lat3, lon3, DEFAULT_ALTITUDE},
        {lat4, lon4, DEFAULT_ALTITUDE}
    };

    bool result = mission_planner->load_perimeter(corners);

    if (result) {
        emit waypointsUpdated();
    } else {
        emit errorOccurred("Invalid perimeter coordinates");
    }

    return result;
}

bool QMLBridge::generateMission() {
    bool result = mission_planner->generate_mission();

    if (result) {
        emit waypointsUpdated();
        emit statisticsUpdated();
        emit missionStateChanged();
    } else {
        emit errorOccurred("Failed to generate mission");
    }

    return result;
}

bool QMLBridge::startMission() {
    bool result = mission_planner->start_mission();

    if (result) {
        simulator_waypoint_index = 0;

        // Start simulator if in simulate mode
        if (mission_planner->get_mode() == MissionMode::SIMULATE) {
            simulator_timer->start(100);  // 100ms updates

            // Initialize drone position at first waypoint
            const auto& waypoints = mission_planner->get_waypoints();
            if (!waypoints.empty()) {
                current_drone_lat = waypoints[0].x;
                current_drone_lon = waypoints[0].y;
                current_drone_alt = waypoints[0].z;
            }
        }

        emit missionStateChanged();
    } else {
        emit errorOccurred("Cannot start mission - check state and configuration");
    }

    return result;
}

bool QMLBridge::pauseMission() {
    bool result = mission_planner->pause_mission();

    if (result) {
        if (simulator_timer->isActive()) {
            simulator_timer->stop();
        }
        emit missionStateChanged();
    } else {
        emit errorOccurred("Cannot pause mission");
    }

    return result;
}

bool QMLBridge::resumeMission() {
    bool result = mission_planner->resume_mission();

    if (result) {
        if (mission_planner->get_mode() == MissionMode::SIMULATE) {
            simulator_timer->start();
        }
        emit missionStateChanged();
    } else {
        emit errorOccurred("Cannot resume mission");
    }

    return result;
}

bool QMLBridge::cancelMission() {
    bool result = mission_planner->cancel_mission();

    if (result) {
        if (simulator_timer->isActive()) {
            simulator_timer->stop();
        }
        emit missionStateChanged();
    } else {
        emit errorOccurred("Cannot cancel mission");
    }

    return result;
}

bool QMLBridge::setMode(bool simulate_mode) {
    MissionMode mode = simulate_mode ? MissionMode::SIMULATE : MissionMode::REAL;
    bool result = mission_planner->set_mode(mode);

    if (result) {
        emit modeChanged();
    } else {
        emit errorOccurred("Cannot change mode during mission execution");
    }

    return result;
}

QVariantList QMLBridge::getWaypoints() {
    QVariantList waypoints;

    const auto& wps = mission_planner->get_waypoints();
    for (const auto& wp : wps) {
        QVariantMap point;
        point["seq"] = (int)wp.seq;
        point["latitude"] = wp.x;
        point["longitude"] = wp.y;
        point["altitude"] = wp.z;
        point["command"] = (int)wp.command;
        waypoints.append(point);
    }

    return waypoints;
}

QVariantList QMLBridge::getPerimeterCorners() {
    QVariantList corners;

    const auto& config = mission_planner->get_config();
    for (int i = 0; i < 4; i++) {
        QVariantMap corner;
        corner["latitude"] = config.perimeter.corners[i].latitude;
        corner["longitude"] = config.perimeter.corners[i].longitude;
        corner["index"] = i;
        corners.append(corner);
    }

    return corners;
}

bool QMLBridge::exportMission(const QString& filepath) {
    bool result = mission_planner->export_mission(filepath.toStdString().c_str());

    if (!result) {
        emit errorOccurred("Failed to export mission: " + filepath);
    }

    return result;
}

bool QMLBridge::importMission(const QString& filepath) {
    bool result = mission_planner->import_mission(filepath.toStdString().c_str());

    if (result) {
        emit waypointsUpdated();
        emit statisticsUpdated();
    } else {
        emit errorOccurred("Failed to import mission: " + filepath);
    }

    return result;
}

void QMLBridge::updateDronePosition(double lat, double lon, double alt) {
    current_drone_lat = lat;
    current_drone_lon = lon;
    current_drone_alt = alt;

    emit dronePositionUpdated(lat, lon, alt);
}

QString QMLBridge::getStatusString() {
    MissionState state = mission_planner->get_state();
    int current_wp = mission_planner->get_current_waypoint();
    int total_wps = mission_planner->get_waypoints().size();

    QString status = QString("State: %1 | WP: %2/%3 | Mode: %4")
        .arg(stateToString(state))
        .arg(current_wp)
        .arg(total_wps)
        .arg(getMissionState());

    return status;
}

double QMLBridge::getMissionProgress() {
    int total = mission_planner->get_waypoints().size();
    if (total == 0) return 0.0;

    return (double)mission_planner->get_current_waypoint() / (double)total * 100.0;
}

QString QMLBridge::getMissionState() const {
    return stateToString(mission_planner->get_state());
}

int QMLBridge::getCurrentWaypoint() const {
    return mission_planner->get_current_waypoint();
}

int QMLBridge::getTotalWaypoints() const {
    return mission_planner->get_waypoints().size();
}

double QMLBridge::getEstimatedTime() const {
    auto stats = mission_planner->get_statistics();
    return stats.estimated_time;
}

double QMLBridge::getTotalDistance() const {
    auto stats = mission_planner->get_statistics();
    return stats.total_distance;
}

double QMLBridge::getPerimeterArea() const {
    auto stats = mission_planner->get_statistics();
    return stats.perimeter_area;
}

QString QMLBridge::getExecutionMode() const {
    return (mission_planner->get_mode() == MissionMode::SIMULATE) ? "Simulate" : "Real";
}

void QMLBridge::onSimulatorTick() {
    // Simulate drone movement at 100ms intervals
    // Total simulation speed: 10x (10 seconds per real second)

    if (mission_planner->get_state() != MissionState::EXECUTING) {
        simulator_timer->stop();
        return;
    }

    const auto& waypoints = mission_planner->get_waypoints();
    if (waypoints.empty()) return;

    // Move towards next waypoint
    int current = mission_planner->get_current_waypoint();
    if (current < (int)waypoints.size()) {
        const auto& target = waypoints[current];

        // Calculate distance to target
        double dlat = target.x - current_drone_lat;
        double dlon = target.y - current_drone_lon;
        double distance = sqrt(dlat * dlat + dlon * dlon);

        // Speed: 0.1 degrees per second (simplified)
        double move_speed = 0.1;

        if (distance < move_speed) {
            // Reached waypoint
            current++;
            if (current >= (int)waypoints.size()) {
                mission_planner->update_current_waypoint(current - 1);
                emit missionStateChanged();
                simulator_timer->stop();
                return;
            }
            mission_planner->update_current_waypoint(current);
            emit waypointChanged();
        } else {
            // Move towards target
            double ratio = move_speed / distance;
            current_drone_lat += dlat * ratio;
            current_drone_lon += dlon * ratio;
            current_drone_alt = target.z;  // Maintain altitude
        }

        emit dronePositionUpdated(current_drone_lat, current_drone_lon, current_drone_alt);
        emit missionProgress(getMissionProgress());
    }
}

QString QMLBridge::stateToString(MissionState state) const {
    switch (state) {
        case MissionState::IDLE: return "Idle";
        case MissionState::PLANNING: return "Planning";
        case MissionState::READY: return "Ready";
        case MissionState::EXECUTING: return "Executing";
        case MissionState::PAUSED: return "Paused";
        case MissionState::COMPLETED: return "Completed";
        case MissionState::ERROR: return "Error";
        default: return "Unknown";
    }
}
