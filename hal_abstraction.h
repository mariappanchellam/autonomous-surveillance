// hal_abstraction.h - Hardware Abstraction Layer
// Single API for both Simulate and Real modes

#pragma once

#include "mission_config.h"
#include <stdint.h>

// Compile-time mode selection
// Build with: cmake -DMISSION_MODE=SIMULATE  or  cmake -DMISSION_MODE=REAL

#define MODE_SIMULATE 0
#define MODE_REAL 1

// Motor control structure
struct MotorCommand {
    float throttle;     // 0.0 to 1.0
    float roll;         // -1.0 to 1.0
    float pitch;        // -1.0 to 1.0
    float yaw;          // -1.0 to 1.0
};

// Flight state feedback
struct FlightState {
    double latitude;
    double longitude;
    float altitude;     // meters above home
    float roll;         // degrees
    float pitch;        // degrees
    float yaw;          // degrees
    float vx;           // velocity x m/s
    float vy;           // velocity y m/s
    float vz;           // velocity z m/s
    uint8_t mode;       // Flight mode
    uint8_t armed;      // 1 if armed
};

/**
 * Hardware Abstraction Layer Interface
 * Provides unified API for motor control and flight management
 */
class HardwareAbstraction {
public:
    virtual ~HardwareAbstraction() {}

    /**
     * Initialize hardware/simulator
     */
    virtual bool initialize() = 0;

    /**
     * Shutdown hardware/simulator
     */
    virtual void shutdown() = 0;

    /**
     * Arm/disarm motors
     */
    virtual bool arm() = 0;
    virtual bool disarm() = 0;

    /**
     * Send motor command
     */
    virtual bool set_motor_command(const MotorCommand& cmd) = 0;

    /**
     * Get current flight state
     */
    virtual bool get_flight_state(FlightState& state) = 0;

    /**
     * Set target position (for autonomous flight)
     */
    virtual bool set_target_position(double lat, double lon, float altitude) = 0;

    /**
     * Set target velocity
     */
    virtual bool set_target_velocity(float vx, float vy, float vz) = 0;

    /**
     * Set flight mode
     * 0=Stabilize, 1=Loiter, 2=Guided, 3=Auto, etc.
     */
    virtual bool set_flight_mode(uint8_t mode) = 0;

    /**
     * Is drone ready for flight
     */
    virtual bool is_ready() = 0;

    /**
     * Get battery percentage
     */
    virtual float get_battery_percentage() = 0;

    /**
     * Take off to altitude
     */
    virtual bool takeoff(float altitude) = 0;

    /**
     * Land
     */
    virtual bool land() = 0;

    /**
     * Go to waypoint
     */
    virtual bool goto_waypoint(const Waypoint& wp) = 0;

    /**
     * Update loop (call regularly)
     */
    virtual void update() = 0;

    /**
     * Get mode name
     */
    virtual const char* get_mode_name() = 0;
};

/**
 * Factory function to get appropriate HAL implementation
 */
HardwareAbstraction* create_hardware_abstraction();

