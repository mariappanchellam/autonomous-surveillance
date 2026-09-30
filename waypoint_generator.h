// waypoint_generator.h - Generate waypoints for perimeter surveillance

#pragma once

#include "mission_config.h"
#include "perimeter_handler.h"
#include <vector>

class WaypointGenerator {
public:
    WaypointGenerator();

    /**
     * Generate lawnmower grid pattern within perimeter
     * Creates back-and-forth pattern for efficient coverage
     */
    bool generate_grid_pattern(const PerimeterConfig& perimeter,
                               double grid_spacing);

    /**
     * Generate circular pattern around perimeter
     */
    bool generate_circular_pattern(const PerimeterConfig& perimeter,
                                   double radius_offset);

    /**
     * Get generated waypoints
     */
    const std::vector<Waypoint>& get_waypoints() const;

    /**
     * Get number of waypoints
     */
    size_t waypoint_count() const;

    /**
     * Get total mission distance (meters)
     */
    double get_total_distance() const;

    /**
     * Get estimated mission time (seconds)
     */
    double get_estimated_time(float speed) const;

    /**
     * Clear all waypoints
     */
    void clear();

    /**
     * Add a single waypoint (for custom mission building)
     */
    bool add_waypoint(const Coordinate& position, float param1 = 0.0f);

    /**
     * Add takeoff waypoint
     */
    bool add_takeoff(const Coordinate& location, float altitude);

    /**
     * Add return-to-launch waypoint
     */
    bool add_rtl();

private:
    std::vector<Waypoint> waypoints;
    PerimeterHandler perimeter_handler;

    /**
     * Create a single waypoint structure
     */
    Waypoint create_waypoint(uint16_t seq, uint16_t command,
                            const Coordinate& position, float param = 0.0f);

    /**
     * Calculate grid bounds based on perimeter
     */
    struct GridBounds {
        double min_x, max_x, min_y, max_y;
        double grid_spacing;
    };
    GridBounds calculate_grid_bounds(const PerimeterConfig& perimeter,
                                     double spacing);
};

