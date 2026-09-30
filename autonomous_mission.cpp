// autonomous_mission.cpp - Implementation

#include "autonomous_mission.h"
#include <cmath>
#include <stdio.h>

AutonomousMission::AutonomousMission()
    : hardware(nullptr), current_waypoint_idx(0), in_flight(false) {
}

AutonomousMission::~AutonomousMission() {
    if (in_flight) {
        abort_mission();
    }
    if (hardware) {
        delete hardware;
    }
}

bool AutonomousMission::initialize() {
    printf("=== Autonomous Mission System v1.0 ===\n");

    // Create appropriate hardware abstraction
    hardware = create_hardware_abstraction();
    if (!hardware) {
        fprintf(stderr, "ERROR: Failed to create hardware abstraction\n");
        return false;
    }

    if (!hardware->initialize()) {
        fprintf(stderr, "ERROR: Failed to initialize hardware\n");
        return false;
    }

    return true;
}

bool AutonomousMission::load_perimeter(const Coordinate corners[4], float altitude) {
    if (!hardware) return false;

    printf("Loading perimeter...\n");
    return mission_planner.load_perimeter(corners, altitude);
}

bool AutonomousMission::plan_mission() {
    if (!hardware) return false;

    printf("Planning mission...\n");
    if (!mission_planner.generate_mission()) {
        fprintf(stderr, "ERROR: Mission planning failed\n");
        return false;
    }

    auto stats = mission_planner.get_statistics();
    printf("Mission planned:\n");
    printf("  Waypoints: %d\n", stats.total_waypoints);
    printf("  Distance: %.2f km\n", stats.total_distance / 1000.0);
    printf("  Est. Time: %.1f minutes\n", stats.estimated_time / 60.0);
    printf("  Area: %.2f km²\n", stats.perimeter_area / 1e6);

    return true;
}

bool AutonomousMission::start_mission() {
    if (!hardware) return false;
    if (mission_planner.get_state() != MissionState::READY) {
        fprintf(stderr, "ERROR: Mission not ready to start\n");
        return false;
    }

    printf("Starting mission...\n");

    // Arm drone
    if (!hardware->arm()) {
        fprintf(stderr, "ERROR: Failed to arm drone\n");
        return false;
    }

    // Set flight mode to GUIDED (autonomous)
    if (!hardware->set_flight_mode(2)) {  // 2 = GUIDED
        fprintf(stderr, "ERROR: Failed to set GUIDED mode\n");
        return false;
    }

    // Takeoff
    const auto& waypoints = mission_planner.get_waypoints();
    if (waypoints.empty()) {
        fprintf(stderr, "ERROR: No waypoints to execute\n");
        return false;
    }

    float takeoff_alt = waypoints[0].z;  // First waypoint altitude
    printf("Taking off to %.1f meters\n", takeoff_alt);

    if (!hardware->takeoff(takeoff_alt)) {
        fprintf(stderr, "ERROR: Takeoff failed\n");
        return false;
    }

    in_flight = true;
    current_waypoint_idx = 0;
    mission_planner.start_mission();

    printf("Mission started!\n");
    return true;
}

bool AutonomousMission::pause_mission() {
    if (!in_flight) return false;

    printf("Mission paused\n");
    return mission_planner.pause_mission();
}

bool AutonomousMission::resume_mission() {
    if (!in_flight) return false;

    printf("Mission resumed\n");
    return mission_planner.resume_mission();
}

bool AutonomousMission::abort_mission() {
    printf("Aborting mission...\n");

    if (!in_flight) return true;

    // Land drone
    if (hardware) {
        hardware->land();
    }

    in_flight = false;
    mission_planner.cancel_mission();

    printf("Mission aborted\n");
    return true;
}

MissionState AutonomousMission::get_state() const {
    return mission_planner.get_state();
}

FlightState AutonomousMission::get_flight_state() const {
    FlightState state = {};
    if (hardware) {
        hardware->get_flight_state(state);
    }
    return state;
}

int AutonomousMission::get_current_waypoint_index() const {
    return current_waypoint_idx;
}

AutonomousMission::MissionStats AutonomousMission::get_statistics() const {
    auto planner_stats = mission_planner.get_statistics();

    MissionStats stats = {};
    stats.total_distance = planner_stats.total_distance;
    stats.estimated_time = planner_stats.estimated_time;
    stats.total_waypoints = planner_stats.total_waypoints;
    stats.current_waypoint = current_waypoint_idx;
    stats.progress_percent = stats.total_waypoints > 0 ?
        (current_waypoint_idx / (float)stats.total_waypoints * 100.0f) : 0.0f;
    stats.battery_percent = hardware ? hardware->get_battery_percentage() : 0.0f;

    return stats;
}

void AutonomousMission::update() {
    if (!hardware || !in_flight) return;

    // Update hardware (physics simulation, sensor reading)
    hardware->update();

    // Check if current waypoint reached
    if (waypoint_reached()) {
        execute_next_waypoint();
    }

    // Check mission completion
    const auto& waypoints = mission_planner.get_waypoints();
    if (current_waypoint_idx >= (int)waypoints.size()) {
        printf("Mission completed!\n");
        abort_mission();
    }
}

const char* AutonomousMission::get_mode_name() const {
    return hardware ? hardware->get_mode_name() : "UNKNOWN";
}

void AutonomousMission::execute_next_waypoint() {
    const auto& waypoints = mission_planner.get_waypoints();

    if (current_waypoint_idx >= (int)waypoints.size()) {
        printf("All waypoints executed\n");
        return;
    }

    const Waypoint& wp = waypoints[current_waypoint_idx];

    printf("Executing waypoint %d: (%.6f, %.6f) @ %.1f m\n",
           current_waypoint_idx, wp.x, wp.y, wp.z);

    if (hardware->goto_waypoint(wp)) {
        current_waypoint_idx++;
        mission_planner.update_current_waypoint(current_waypoint_idx);
    }
}

bool AutonomousMission::waypoint_reached() {
    if (!hardware) return false;
    if (current_waypoint_idx <= 0) return false;

    const auto& waypoints = mission_planner.get_waypoints();
    if (current_waypoint_idx >= (int)waypoints.size()) return false;

    const Waypoint& current_wp = waypoints[current_waypoint_idx - 1];
    FlightState state = get_flight_state();

    // Calculate distance to waypoint
    double dlat = current_wp.x - state.latitude;
    double dlon = current_wp.y - state.longitude;
    float dalt = current_wp.z - state.altitude;

    // Convert to meters (simplified)
    double dx = dlat * 111320.0;
    double dy = dlon * 111320.0;

    double distance = sqrt(dx*dx + dy*dy + dalt*dalt);

    // Waypoint reached when within 2 meters
    return distance < 2.0;
}
