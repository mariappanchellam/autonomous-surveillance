// autonomous_mission.h - Main Mission Control (uses HAL abstraction)

#pragma once

#include "mission_config.h"
#include "mission_planner.h"
#include "hal_abstraction.h"
#include <vector>

/**
 * Main Autonomous Mission Controller
 * Coordinates mission planning with hardware control
 */
class AutonomousMission {
public:
    AutonomousMission();
    ~AutonomousMission();

    /**
     * Initialize mission system (creates appropriate HAL)
     */
    bool initialize();

    /**
     * Load factory perimeter
     */
    bool load_perimeter(const Coordinate corners[4], float altitude = DEFAULT_ALTITUDE);

    /**
     * Plan mission (generate waypoints)
     */
    bool plan_mission();

    /**
     * Start executing mission
     */
    bool start_mission();

    /**
     * Pause mission
     */
    bool pause_mission();

    /**
     * Resume mission
     */
    bool resume_mission();

    /**
     * Abort mission
     */
    bool abort_mission();

    /**
     * Get current mission state
     */
    MissionState get_state() const;

    /**
     * Get flight state (position, attitude, etc.)
     */
    FlightState get_flight_state() const;

    /**
     * Get current waypoint index
     */
    int get_current_waypoint_index() const;

    /**
     * Get mission statistics
     */
    struct MissionStats {
        double total_distance;
        double estimated_time;
        int total_waypoints;
        int current_waypoint;
        float progress_percent;
        float battery_percent;
    };
    MissionStats get_statistics() const;

    /**
     * Update mission (call regularly from main loop)
     */
    void update();

    /**
     * Get compilation mode
     */
    const char* get_mode_name() const;

private:
    HardwareAbstraction* hardware;  // Hardware/Simulator abstraction
    MissionPlanner mission_planner;

    int current_waypoint_idx;
    bool in_flight;

    /**
     * Execute next waypoint
     */
    void execute_next_waypoint();

    /**
     * Check if drone reached target waypoint
     */
    bool waypoint_reached();
};

