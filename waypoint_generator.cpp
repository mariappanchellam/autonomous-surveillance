// waypoint_generator.cpp - Implementation

#include "waypoint_generator.h"
#include <cmath>
#include <algorithm>

#define EARTH_RADIUS_M 6371000.0

// MAVLink command IDs
#define MAV_CMD_NAV_WAYPOINT 16
#define MAV_CMD_NAV_TAKEOFF 22
#define MAV_CMD_NAV_RETURN_TO_LAUNCH 20
#define MAV_FRAME_GLOBAL_RELATIVE_ALT 3

WaypointGenerator::WaypointGenerator() {
    waypoints.reserve(MAX_WAYPOINTS);
}

bool WaypointGenerator::generate_grid_pattern(const PerimeterConfig& perimeter,
                                               double grid_spacing) {
    clear();
    perimeter_handler.set_perimeter(perimeter.corners);

    GridBounds bounds = calculate_grid_bounds(perimeter, grid_spacing);

    // Reference point for coordinate conversion
    Coordinate ref = perimeter.corners[0];

    // Generate horizontal lines (lawnmower pattern)
    double y = bounds.min_y;
    bool forward = true;

    while (y <= bounds.max_y && waypoints.size() < MAX_WAYPOINTS - 2) {
        if (forward) {
            for (double x = bounds.min_x; x <= bounds.max_x && waypoints.size() < MAX_WAYPOINTS - 2;
                 x += grid_spacing) {

                // Check if point is inside perimeter
                if (perimeter_handler.point_in_perimeter(x, y)) {
                    Coordinate wp = PerimeterHandler::xy_to_lat_lon(ref, x, y);
                    wp.altitude = perimeter.altitude;
                    add_waypoint(wp);
                }
            }
        } else {
            for (double x = bounds.max_x; x >= bounds.min_x && waypoints.size() < MAX_WAYPOINTS - 2;
                 x -= grid_spacing) {

                if (perimeter_handler.point_in_perimeter(x, y)) {
                    Coordinate wp = PerimeterHandler::xy_to_lat_lon(ref, x, y);
                    wp.altitude = perimeter.altitude;
                    add_waypoint(wp);
                }
            }
        }

        y += grid_spacing;
        forward = !forward;
    }

    return !waypoints.empty();
}

bool WaypointGenerator::generate_circular_pattern(const PerimeterConfig& perimeter,
                                                   double radius_offset) {
    clear();
    perimeter_handler.set_perimeter(perimeter.corners);

    // Calculate center of perimeter
    double center_lat = 0.0, center_lon = 0.0;
    for (int i = 0; i < 4; i++) {
        center_lat += perimeter.corners[i].latitude;
        center_lon += perimeter.corners[i].longitude;
    }
    center_lat /= 4.0;
    center_lon /= 4.0;

    // Generate circular waypoints
    int num_points = 36;  // One point every 10 degrees
    double angle_step = 360.0 / num_points;

    Coordinate center = {center_lat, center_lon, perimeter.altitude};
    double radius_rad = (radius_offset / EARTH_RADIUS_M) * (180.0 / M_PI);

    for (int i = 0; i < num_points && waypoints.size() < MAX_WAYPOINTS - 1; i++) {
        double angle = i * angle_step * M_PI / 180.0;

        Coordinate wp;
        wp.latitude = center.latitude + radius_rad * sin(angle);
        wp.longitude = center.longitude + radius_rad * cos(angle) /
                      cos(center.latitude * M_PI / 180.0);
        wp.altitude = perimeter.altitude;

        add_waypoint(wp);
    }

    return !waypoints.empty();
}

const std::vector<Waypoint>& WaypointGenerator::get_waypoints() const {
    return waypoints;
}

size_t WaypointGenerator::waypoint_count() const {
    return waypoints.size();
}

double WaypointGenerator::get_total_distance() const {
    if (waypoints.size() < 2) return 0.0;

    double total = 0.0;
    for (size_t i = 0; i < waypoints.size() - 1; i++) {
        Coordinate p1 = {waypoints[i].x, waypoints[i].y, waypoints[i].z};
        Coordinate p2 = {waypoints[i+1].x, waypoints[i+1].y, waypoints[i+1].z};

        total += PerimeterHandler::haversine_distance(p1, p2);
    }

    return total;
}

double WaypointGenerator::get_estimated_time(float speed) const {
    if (speed <= 0) return 0.0;
    return get_total_distance() / speed;
}

void WaypointGenerator::clear() {
    waypoints.clear();
}

bool WaypointGenerator::add_waypoint(const Coordinate& position, float param1) {
    if (waypoints.size() >= MAX_WAYPOINTS) {
        return false;
    }

    Waypoint wp = create_waypoint(waypoints.size(), MAV_CMD_NAV_WAYPOINT, position, param1);
    waypoints.push_back(wp);
    return true;
}

bool WaypointGenerator::add_takeoff(const Coordinate& location, float altitude) {
    if (waypoints.size() >= MAX_WAYPOINTS) {
        return false;
    }

    Coordinate takeoff_pos = location;
    takeoff_pos.altitude = altitude;

    Waypoint wp = create_waypoint(waypoints.size(), MAV_CMD_NAV_TAKEOFF, takeoff_pos, altitude);
    wp.param1 = 0.0f;  // Pitch
    waypoints.insert(waypoints.begin(), wp);

    // Update sequence numbers
    for (size_t i = 0; i < waypoints.size(); i++) {
        waypoints[i].seq = i;
    }

    return true;
}

bool WaypointGenerator::add_rtl() {
    if (waypoints.size() >= MAX_WAYPOINTS) {
        return false;
    }

    Coordinate dummy = {0, 0, 0};
    Waypoint wp = create_waypoint(waypoints.size(), MAV_CMD_NAV_RETURN_TO_LAUNCH, dummy);
    waypoints.push_back(wp);
    return true;
}

Waypoint WaypointGenerator::create_waypoint(uint16_t seq, uint16_t command,
                                            const Coordinate& position, float param) {
    Waypoint wp = {};
    wp.seq = seq;
    wp.frame = MAV_FRAME_GLOBAL_RELATIVE_ALT;
    wp.command = command;
    wp.current = (seq == 0) ? 1 : 0;
    wp.autocontinue = 1;
    wp.param1 = param;
    wp.param2 = 0.0f;
    wp.param3 = 0.0f;
    wp.param4 = 0.0f;
    wp.x = position.latitude;
    wp.y = position.longitude;
    wp.z = position.altitude;
    return wp;
}

WaypointGenerator::GridBounds WaypointGenerator::calculate_grid_bounds(
    const PerimeterConfig& perimeter, double spacing) {

    GridBounds bounds;
    Coordinate ref = perimeter.corners[0];

    // Convert all corners to local coordinates
    double min_x = 1e9, max_x = -1e9;
    double min_y = 1e9, max_y = -1e9;

    for (int i = 0; i < 4; i++) {
        double x, y;
        PerimeterHandler::lat_lon_to_xy(ref, perimeter.corners[i], x, y);
        min_x = std::min(min_x, x);
        max_x = std::max(max_x, x);
        min_y = std::min(min_y, y);
        max_y = std::max(max_y, y);
    }

    // Add margin
    double margin = spacing / 2.0;
    bounds.min_x = min_x - margin;
    bounds.max_x = max_x + margin;
    bounds.min_y = min_y - margin;
    bounds.max_y = max_y + margin;
    bounds.grid_spacing = spacing;

    return bounds;
}
