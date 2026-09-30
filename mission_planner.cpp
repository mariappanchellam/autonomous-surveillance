// mission_planner.cpp - Implementation

#include "mission_planner.h"
#include <cstring>
#include <fstream>
#include <ctime>

MissionPlanner::MissionPlanner()
    : current_waypoint_index(0), last_perimeter_update(0) {
    memset(&config, 0, sizeof(config));
    config.state = MissionState::IDLE;
}

MissionPlanner::~MissionPlanner() {
    cancel_mission();
}

bool MissionPlanner::initialize(MissionMode mode) {
    config.mode = mode;
    config.state = MissionState::IDLE;
    config.auto_execute = false;
    config.battery_threshold = 20.0f;
    return true;
}

bool MissionPlanner::load_perimeter(const Coordinate corners[4], float altitude,
                                     double grid_spacing) {
    if (!corners) return false;

    // Validate and set perimeter
    if (!perimeter_handler.set_perimeter(corners)) {
        config.state = MissionState::ERROR;
        return false;
    }

    // Store perimeter configuration
    config.perimeter = perimeter_handler.get_perimeter_config();
    config.perimeter.altitude = altitude;
    config.perimeter.grid_spacing = grid_spacing;
    config.timestamp = time(nullptr);
    config.state = MissionState::PLANNING;

    return true;
}

bool MissionPlanner::generate_mission() {
    if (config.state == MissionState::ERROR) {
        return false;
    }

    // Generate waypoints
    if (!waypoint_generator.generate_grid_pattern(config.perimeter,
                                                   config.perimeter.grid_spacing)) {
        config.state = MissionState::ERROR;
        return false;
    }

    // Add takeoff waypoint at first corner
    if (!waypoint_generator.add_takeoff(config.perimeter.corners[0],
                                        config.perimeter.altitude)) {
        config.state = MissionState::ERROR;
        return false;
    }

    // Add return-to-launch waypoint at the end
    if (!waypoint_generator.add_rtl()) {
        config.state = MissionState::ERROR;
        return false;
    }

    config.state = MissionState::READY;
    current_waypoint_index = 0;
    return true;
}

bool MissionPlanner::start_mission() {
    if (config.state != MissionState::READY && config.state != MissionState::PAUSED) {
        return false;
    }

    if (config.mode == MissionMode::REAL) {
        if (!upload_to_autopilot()) {
            return false;
        }
    }

    config.state = MissionState::EXECUTING;
    current_waypoint_index = 0;
    return true;
}

bool MissionPlanner::pause_mission() {
    if (config.state != MissionState::EXECUTING) {
        return false;
    }

    config.state = MissionState::PAUSED;
    return true;
}

bool MissionPlanner::resume_mission() {
    if (config.state != MissionState::PAUSED) {
        return false;
    }

    config.state = MissionState::EXECUTING;
    return true;
}

bool MissionPlanner::cancel_mission() {
    if (config.state == MissionState::IDLE || config.state == MissionState::COMPLETED) {
        return false;
    }

    waypoint_generator.clear();
    config.state = MissionState::IDLE;
    current_waypoint_index = 0;
    return true;
}

MissionState MissionPlanner::get_state() const {
    return config.state;
}

const MissionConfig& MissionPlanner::get_config() const {
    return config;
}

const std::vector<Waypoint>& MissionPlanner::get_waypoints() const {
    return waypoint_generator.get_waypoints();
}

int MissionPlanner::get_current_waypoint() const {
    return current_waypoint_index;
}

void MissionPlanner::update_current_waypoint(int index) {
    const auto& waypoints = waypoint_generator.get_waypoints();
    if (index >= 0 && index < (int)waypoints.size()) {
        current_waypoint_index = index;

        // Check if mission is complete
        if (index >= (int)waypoints.size() - 1) {
            config.state = MissionState::COMPLETED;
        }
    }
}

MissionPlanner::MissionStats MissionPlanner::get_statistics() const {
    MissionStats stats = {};
    stats.total_distance = waypoint_generator.get_total_distance();
    stats.estimated_time = waypoint_generator.get_estimated_time(config.perimeter.speed);
    stats.total_waypoints = waypoint_generator.waypoint_count();
    stats.perimeter_area = perimeter_handler.calculate_area();
    return stats;
}

bool MissionPlanner::set_mode(MissionMode mode) {
    if (config.state == MissionState::EXECUTING) {
        return false;  // Cannot change mode during execution
    }

    config.mode = mode;
    return true;
}

MissionMode MissionPlanner::get_mode() const {
    return config.mode;
}

bool MissionPlanner::queue_mission(time_t execute_at) {
    if (config.state != MissionState::READY) {
        return false;
    }

    MissionConfig queued = config;
    if (execute_at == 0) {
        execute_at = time(nullptr) + 3600;  // Default: 1 hour from now
    }
    queued.timestamp = execute_at;

    mission_queue.push(queued);
    return true;
}

size_t MissionPlanner::get_queue_size() const {
    return mission_queue.size();
}

bool MissionPlanner::export_mission(const char* filename) {
    if (!filename) return false;

    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Write mission header
    const auto& waypoints = waypoint_generator.get_waypoints();
    uint16_t wp_count = waypoints.size();
    file.write(reinterpret_cast<char*>(&wp_count), sizeof(wp_count));

    // Write mission config
    file.write(reinterpret_cast<char*>(&config), sizeof(config));

    // Write waypoints
    for (const auto& wp : waypoints) {
        file.write(reinterpret_cast<const char*>(&wp), sizeof(wp));
    }

    file.close();
    return file.good();
}

bool MissionPlanner::import_mission(const char* filename) {
    if (!filename) return false;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Read mission header
    uint16_t wp_count = 0;
    file.read(reinterpret_cast<char*>(&wp_count), sizeof(wp_count));

    // Read mission config
    file.read(reinterpret_cast<char*>(&config), sizeof(config));

    // Read waypoints
    waypoint_generator.clear();
    for (uint16_t i = 0; i < wp_count; i++) {
        Waypoint wp = {};
        file.read(reinterpret_cast<char*>(&wp), sizeof(wp));
        // Add to generator
    }

    file.close();
    config.state = MissionState::READY;
    return file.good();
}

bool MissionPlanner::upload_to_autopilot() {
    // TODO: Implement actual ArduPilot communication
    // This would use MAVLink protocol to send waypoints
    // For now, this is a placeholder
    return (config.mode == MissionMode::REAL);
}

bool MissionPlanner::validate_mission() {
    if (waypoint_generator.waypoint_count() < 2) {
        return false;
    }

    double mission_time = waypoint_generator.get_estimated_time(config.perimeter.speed);
    if (mission_time <= 0) {
        return false;
    }

    // Check if perimeter is valid
    auto bounds = perimeter_handler.get_bounds();
    if (bounds.min_lat == bounds.max_lat || bounds.min_lon == bounds.max_lon) {
        return false;
    }

    return true;
}
