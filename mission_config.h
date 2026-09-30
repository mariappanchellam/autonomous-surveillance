// mission_config.h - Configuration constants for autonomous mission system

#pragma once

#include <stdint.h>

// Mission parameters
#define MAX_WAYPOINTS 500
#define MAX_PERIMETER_POINTS 4
#define DEFAULT_ALTITUDE 50.0f  // meters
#define DEFAULT_SPEED 10.0f     // m/s

// Coordinate system
struct Coordinate {
    double latitude;
    double longitude;
    float altitude;
};

// Mission modes
enum class MissionMode {
    SIMULATE = 0,
    REAL = 1
};

// Mission state
enum class MissionState {
    IDLE = 0,
    PLANNING = 1,
    READY = 2,
    EXECUTING = 3,
    PAUSED = 4,
    COMPLETED = 5,
    ERROR = 6
};

// Perimeter configuration
struct PerimeterConfig {
    Coordinate corners[MAX_PERIMETER_POINTS];  // 4 corners of square
    uint8_t corner_count;
    double grid_spacing;  // meters between waypoints
    float altitude;
    float speed;
};

// Mission parameters
struct MissionConfig {
    PerimeterConfig perimeter;
    MissionMode mode;
    MissionState state;
    uint32_t mission_id;
    uint32_t timestamp;  // Hour when perimeter was updated
    bool auto_execute;
    float battery_threshold;  // percentage
};

// Waypoint structure (compatible with ArduPilot)
struct Waypoint {
    uint16_t seq;
    uint8_t frame;           // 0=global, 1=local, etc.
    uint16_t command;        // MAV_CMD_NAV_WAYPOINT, etc.
    uint8_t current;
    uint8_t autocontinue;
    float param1, param2, param3, param4;
    double x, y, z;          // latitude, longitude, altitude
};

