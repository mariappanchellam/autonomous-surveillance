// perimeter_handler.cpp - Implementation

#include "perimeter_handler.h"
#include <algorithm>
#include <cstring>

#define DEG_TO_RAD (M_PI / 180.0)
#define RAD_TO_DEG (180.0 / M_PI)
#define EARTH_RADIUS_M 6371000.0

PerimeterHandler::PerimeterHandler() : valid(false) {
    memset(&config, 0, sizeof(config));
}

bool PerimeterHandler::set_perimeter(const Coordinate corners[4]) {
    if (!corners) return false;

    // Copy corners to config
    for (int i = 0; i < 4; i++) {
        config.corners[i] = corners[i];
    }
    config.corner_count = 4;

    // Default values
    config.grid_spacing = 30.0;  // 30m spacing for surveillance
    config.altitude = DEFAULT_ALTITUDE;
    config.speed = DEFAULT_SPEED;

    return validate_perimeter();
}

bool PerimeterHandler::validate_perimeter() {
    if (config.corner_count != 4) {
        return false;
    }

    // Check if all coordinates are valid
    for (int i = 0; i < 4; i++) {
        const Coordinate& c = config.corners[i];
        if (c.latitude < -90.0 || c.latitude > 90.0 ||
            c.longitude < -180.0 || c.longitude > 180.0) {
            return false;
        }
    }

    // Calculate distances between corners
    double distances[4];
    for (int i = 0; i < 4; i++) {
        int next = (i + 1) % 4;
        distances[i] = haversine_distance(config.corners[i], config.corners[next]);
    }

    // For a square, adjacent sides should be approximately equal
    // Check if it forms a reasonable quadrilateral
    double min_dist = *std::min_element(distances, distances + 4);
    double max_dist = *std::max_element(distances, distances + 4);

    // Allow 20% variance in side lengths (for slight irregularities)
    if (min_dist > 0 && max_dist / min_dist > 1.2) {
        // Try diagonal check for square validation
        double diag1 = haversine_distance(config.corners[0], config.corners[2]);
        double diag2 = haversine_distance(config.corners[1], config.corners[3]);

        // Diagonals should be approximately equal for a square
        if (diag1 > 0 && diag2 / diag1 > 1.1) {
            return false;
        }
    }

    valid = true;
    return true;
}

const PerimeterConfig& PerimeterHandler::get_perimeter_config() const {
    return config;
}

double PerimeterHandler::calculate_area() const {
    if (!valid) return 0.0;

    // Convert to projected coordinates for area calculation
    std::vector<std::pair<double, double>> points;
    double ref_lat = (config.corners[0].latitude + config.corners[2].latitude) / 2.0;
    double ref_lon = (config.corners[0].longitude + config.corners[2].longitude) / 2.0;
    Coordinate ref = {ref_lat, ref_lon, 0};

    for (int i = 0; i < 4; i++) {
        double x, y;
        lat_lon_to_xy(ref, config.corners[i], x, y);
        points.push_back({x, y});
    }

    return shoelace_area(points);
}

PerimeterHandler::Bounds PerimeterHandler::get_bounds() {
    Bounds bounds = {90.0, -90.0, 180.0, -180.0};

    for (int i = 0; i < 4; i++) {
        bounds.min_lat = std::min(bounds.min_lat, config.corners[i].latitude);
        bounds.max_lat = std::max(bounds.max_lat, config.corners[i].latitude);
        bounds.min_lon = std::min(bounds.min_lon, config.corners[i].longitude);
        bounds.max_lon = std::max(bounds.max_lon, config.corners[i].longitude);
    }

    return bounds;
}

double PerimeterHandler::haversine_distance(const Coordinate& p1, const Coordinate& p2) {
    double lat1 = p1.latitude * DEG_TO_RAD;
    double lat2 = p2.latitude * DEG_TO_RAD;
    double delta_lat = (p2.latitude - p1.latitude) * DEG_TO_RAD;
    double delta_lon = (p2.longitude - p1.longitude) * DEG_TO_RAD;

    double a = sin(delta_lat / 2.0) * sin(delta_lat / 2.0) +
               cos(lat1) * cos(lat2) * sin(delta_lon / 2.0) * sin(delta_lon / 2.0);
    double c = 2.0 * atan2(sqrt(a), sqrt(1.0 - a));

    return EARTH_RADIUS_M * c;
}

void PerimeterHandler::lat_lon_to_xy(const Coordinate& ref, const Coordinate& point,
                                      double& x, double& y) {
    // Simple equirectangular projection for small areas
    double cos_lat = cos(ref.latitude * DEG_TO_RAD);

    x = (point.longitude - ref.longitude) * DEG_TO_RAD * EARTH_RADIUS_M * cos_lat;
    y = (point.latitude - ref.latitude) * DEG_TO_RAD * EARTH_RADIUS_M;
}

Coordinate PerimeterHandler::xy_to_lat_lon(const Coordinate& ref, double x, double y) {
    double cos_lat = cos(ref.latitude * DEG_TO_RAD);

    double lat = ref.latitude + (y / EARTH_RADIUS_M) * RAD_TO_DEG;
    double lon = ref.longitude + (x / (EARTH_RADIUS_M * cos_lat)) * RAD_TO_DEG;

    return {lat, lon, ref.altitude};
}

bool PerimeterHandler::point_in_perimeter(double x, double y) {
    // Ray casting algorithm
    int intersections = 0;

    for (int i = 0; i < 4; i++) {
        int j = (i + 1) % 4;

        double x1, y1, x2, y2;
        Coordinate ref = {config.corners[0].latitude, config.corners[0].longitude, 0};

        lat_lon_to_xy(ref, config.corners[i], x1, y1);
        lat_lon_to_xy(ref, config.corners[j], x2, y2);

        if ((y1 <= y && y < y2) || (y2 <= y && y < y1)) {
            double x_intersect = x1 + (y - y1) / (y2 - y1) * (x2 - x1);
            if (x < x_intersect) {
                intersections++;
            }
        }
    }

    return intersections % 2 == 1;
}

double PerimeterHandler::shoelace_area(const std::vector<std::pair<double, double>>& points) const {
    if (points.size() < 3) return 0.0;

    double area = 0.0;
    for (size_t i = 0; i < points.size(); i++) {
        size_t j = (i + 1) % points.size();
        area += points[i].first * points[j].second;
        area -= points[j].first * points[i].second;
    }

    return abs(area) / 2.0;
}
