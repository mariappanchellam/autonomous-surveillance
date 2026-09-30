// perimeter_handler.h - Handle perimeter input and validation

#pragma once

#include "mission_config.h"
#include <vector>
#include <cmath>

class PerimeterHandler {
public:
    PerimeterHandler();

    /**
     * Set perimeter corners (4 coordinates for square)
     * Coordinates should be in order: NW, NE, SE, SW
     */
    bool set_perimeter(const Coordinate corners[4]);

    /**
     * Validate perimeter geometry
     * Checks if corners form a reasonable quadrilateral
     */
    bool validate_perimeter();

    /**
     * Get perimeter configuration
     */
    const PerimeterConfig& get_perimeter_config() const;

    /**
     * Calculate perimeter area (m²)
     */
    double calculate_area() const;

    /**
     * Get perimeter bounds (min/max lat/lon)
     */
    struct Bounds {
        double min_lat, max_lat;
        double min_lon, max_lon;
    };
    Bounds get_bounds();

    /**
     * Calculate distance between two points (Haversine formula)
     */
    static double haversine_distance(const Coordinate& p1, const Coordinate& p2);

    /**
     * Convert geographic coordinates to projected coordinates (simple equirectangular)
     * For small areas only - approximation
     */
    static void lat_lon_to_xy(const Coordinate& ref, const Coordinate& point,
                              double& x, double& y);

    /**
     * Convert projected coordinates back to geographic
     */
    static Coordinate xy_to_lat_lon(const Coordinate& ref, double x, double y);

    /**
     * Check if point is inside perimeter (ray casting algorithm)
     */
    bool point_in_perimeter(double x, double y);

private:
    PerimeterConfig config;
    bool valid;

    /**
     * Calculate polygon area using Shoelace formula
     */
    double shoelace_area(const std::vector<std::pair<double, double>>& points) const;
};
