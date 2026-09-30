// hal_simulator.cpp - Simulator Implementation

#include "hal_simulator.h"
#include <stdio.h>
#include <cmath>

#ifdef COMPILE_FOR_SIMULATE

SimulatorImpl::SimulatorImpl()
    : initialized(false), armed(false), current_mode(0),
      battery_level(100.0f), has_target(false) {

    // Initialize state
    current_state = {};
    target_state = {};
    current_command = {};
    last_update = time(nullptr);
}

SimulatorImpl::~SimulatorImpl() {
    shutdown();
}

bool SimulatorImpl::initialize() {
    // Set home position (example: San Francisco)
    current_state.latitude = 37.7749;
    current_state.longitude = -122.4194;
    current_state.altitude = 0.0f;
    current_state.roll = 0.0f;
    current_state.pitch = 0.0f;
    current_state.yaw = 0.0f;
    current_state.vx = 0.0f;
    current_state.vy = 0.0f;
    current_state.vz = 0.0f;
    current_state.armed = 0;
    current_state.mode = 0;

    initialized = true;
    printf("[SIM] Simulator Initialized at (%.4f, %.4f)\n",
           current_state.latitude, current_state.longitude);
    return true;
}

void SimulatorImpl::shutdown() {
    if (armed) {
        disarm();
    }
    initialized = false;
    printf("[SIM] Simulator Shutdown\n");
}

bool SimulatorImpl::arm() {
    if (!initialized) return false;

    armed = true;
    current_state.armed = 1;
    printf("[SIM] Armed - Ready for flight\n");
    return true;
}

bool SimulatorImpl::disarm() {
    if (!initialized) return false;

    armed = false;
    current_state.armed = 0;
    current_state.vx = 0.0f;
    current_state.vy = 0.0f;
    current_state.vz = 0.0f;
    current_command.throttle = 0.0f;
    printf("[SIM] Disarmed\n");
    return true;
}

bool SimulatorImpl::set_motor_command(const MotorCommand& cmd) {
    if (!initialized || !armed) return false;

    current_command = cmd;
    return true;
}

bool SimulatorImpl::get_flight_state(FlightState& state) {
    if (!initialized) return false;

    state = current_state;
    return true;
}

bool SimulatorImpl::set_target_position(double lat, double lon, float altitude) {
    if (!initialized) return false;

    target_lat = lat;
    target_lon = lon;
    target_alt = altitude;
    has_target = true;

    printf("[SIM] Target set: (%.6f, %.6f) @ %.1f m\n", lat, lon, altitude);
    return true;
}

bool SimulatorImpl::set_target_velocity(float vx, float vy, float vz) {
    if (!initialized || !armed) return false;

    // Set desired velocity
    target_state.vx = vx;
    target_state.vy = vy;
    target_state.vz = vz;

    return true;
}

bool SimulatorImpl::set_flight_mode(uint8_t mode) {
    if (!initialized) return false;

    current_mode = mode;
    current_state.mode = mode;

    const char* mode_names[] = {
        "Stabilize", "Loiter", "Guided", "Auto", "Manual"
    };

    if (mode < 5) {
        printf("[SIM] Mode: %s\n", mode_names[mode]);
    }

    return true;
}

bool SimulatorImpl::is_ready() {
    return initialized && armed;
}

float SimulatorImpl::get_battery_percentage() {
    return battery_level;
}

bool SimulatorImpl::takeoff(float altitude) {
    if (!initialized || !armed) return false;

    set_flight_mode(1);  // LOITER mode
    return set_target_position(current_state.latitude, current_state.longitude, altitude);
}

bool SimulatorImpl::land() {
    if (!initialized) return false;

    set_flight_mode(0);  // STABILIZE mode
    has_target = false;
    return true;
}

bool SimulatorImpl::goto_waypoint(const Waypoint& wp) {
    return set_target_position(wp.x, wp.y, wp.z);
}

void SimulatorImpl::update() {
    if (!initialized) return;

    time_t now = time(nullptr);
    float dt = (now - last_update) > 0 ? (now - last_update) : 0.01f;
    last_update = now;

    if (dt > 1.0f) dt = 0.01f;  // Cap dt

    // Simulate physics
    if (armed) {
        simulate_physics(dt);

        // Consume battery
        battery_level -= 0.1f * dt;  // ~10% per second during flight
        if (battery_level < 0) battery_level = 0;
    }

    // Target seeking (if in GUIDED mode)
    if (armed && has_target && current_mode == 2) {  // GUIDED mode
        double dlat = target_lat - current_state.latitude;
        double dlon = target_lon - current_state.longitude;
        float dalt = target_alt - current_state.altitude;

        // Convert to meters
        float dx = dlat * 111320.0f;  // meters per degree latitude
        float dy = dlon * 111320.0f * cosf(current_state.latitude * 3.14159f / 180.0f);

        // Simple proportional controller
        float max_speed = 10.0f;  // m/s
        float distance = sqrtf(dx*dx + dy*dy + dalt*dalt);

        if (distance < 1.0f) {
            // Target reached
            current_state.vx = 0.0f;
            current_state.vy = 0.0f;
            current_state.vz = 0.0f;
            current_state.altitude = target_alt;
            has_target = false;
        } else {
            // Move towards target
            float speed = (distance > 20.0f) ? max_speed : (distance * max_speed / 20.0f);
            current_state.vx = (dx / distance) * speed;
            current_state.vy = (dy / distance) * speed;
            current_state.vz = (dalt / distance) * speed;
        }
    }
}

void SimulatorImpl::simulate_physics(float dt) {
    // Apply motor forces
    apply_motor_forces(dt);

    // Apply gravity
    apply_gravity(dt);

    // Apply drag
    apply_drag(dt);

    // Update position based on velocity
    double lat_per_meter = 1.0 / 111320.0;
    double lon_per_meter = lat_per_meter / cosf(current_state.latitude * 3.14159f / 180.0f);

    current_state.latitude += current_state.vy * lat_per_meter * dt;
    current_state.longitude += current_state.vx * lon_per_meter * dt;
    current_state.altitude += current_state.vz * dt;

    // Altitude bounds
    if (current_state.altitude < 0.0f) {
        current_state.altitude = 0.0f;
        current_state.vz = 0.0f;  // Stop downward velocity
        disarm();  // Auto-disarm on ground
    }

    // Update attitude based on motor commands
    current_state.roll = current_command.roll * 45.0f;   // Max 45 degrees
    current_state.pitch = current_command.pitch * 45.0f;
    current_state.yaw += current_command.yaw * 180.0f * dt;  // Degrees per second
}

void SimulatorImpl::apply_motor_forces(float dt) {
    // Throttle lifts the drone
    float lift = (current_command.throttle - 0.5f) * 20.0f;  // Convert to m/s²
    current_state.vz += lift * dt;
}

void SimulatorImpl::apply_gravity(float dt) {
    const float gravity = 9.81f;  // m/s²

    // Gravity acts on vz
    if (current_state.altitude > 0.0f) {
        current_state.vz -= gravity * dt;
    }
}

void SimulatorImpl::apply_drag(float dt) {
    // Simple drag model (velocity-proportional)
    const float drag_coefficient = 0.1f;

    current_state.vx *= (1.0f - drag_coefficient * dt);
    current_state.vy *= (1.0f - drag_coefficient * dt);
    current_state.vz *= (1.0f - drag_coefficient * dt);
}

#endif // COMPILE_FOR_SIMULATE
