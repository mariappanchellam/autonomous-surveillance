// mission_planner.h - Main mission planner

#pragma once

#include "mission_config.h"
#include "perimeter_handler.h"
#include "waypoint_generator.h"
#include <vector>
#include <queue>
#include <ctime>

class MissionPlanner {
public:
    MissionPlanner();
    ~MissionPlanner();

    /**
     * Initialize mission planner with mode
     */
    bool initialize(MissionMode mode);

    /**
     * Load perimeter (4 coordinates) and generate mission
     */
    bool load_perimeter(const Coordinate corners[4], float altitude = DEFAULT_ALTITUDE,
                        double grid_spacing = 30.0);

    /**
     * Generate mission waypoints based on perimeter
     */
    bool generate_mission();

    /**
     * Start mission execution
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
     * Cancel mission
     */
    bool cancel_mission();

    /**
     * Get current mission state
     */
    MissionState get_state() const;

    /**
     * Get mission configuration
     */
    const MissionConfig& get_config() const;

    /**
     * Get waypoints
     */
    const std::vector<Waypoint>& get_waypoints() const;

    /**
     * Get current waypoint index
     */
    int get_current_waypoint() const;

    /**
     * Update current waypoint (for simulator)
     */
    void update_current_waypoint(int index);

    /**
     * Get mission statistics
     */
    struct MissionStats {
        double total_distance;  // meters
        double estimated_time;  // seconds
        int total_waypoints;
        double perimeter_area;  // m²
    };
    MissionStats get_statistics() const;

    /**
     * Set execution mode (SIMULATE or REAL)
     */
    bool set_mode(MissionMode mode);

    /**
     * Get execution mode
     */
    MissionMode get_mode() const;

    /**
     * Queue mission for scheduled execution
     */
    bool queue_mission(time_t execute_at = 0);

    /**
     * Get mission queue size
     */
    size_t get_queue_size() const;

    /**
     * Export mission to MAVLink format
     */
    bool export_mission(const char* filename);

    /**
     * Import mission from MAVLink format
     */
    bool import_mission(const char* filename);

private:
    MissionConfig config;
    PerimeterHandler perimeter_handler;
    WaypointGenerator waypoint_generator;
    int current_waypoint_index;

    std::queue<MissionConfig> mission_queue;
    time_t last_perimeter_update;

    /**
     * Send waypoints to autopilot
     */
    bool upload_to_autopilot();

    /**
     * Verify mission feasibility
     */
    bool validate_mission();
};

