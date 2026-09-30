// hal_simulator.h - Simulator Implementation (Desktop PC)

#pragma once

#include "hal_abstraction.h"
#include <ctime>
#include <cmath>

#ifdef COMPILE_FOR_SIMULATE

class SimulatorImpl : public HardwareAbstraction {
public:
    SimulatorImpl();
    ~SimulatorImpl();

    bool initialize() override;
    void shutdown() override;

    bool arm() override;
    bool disarm() override;

    bool set_motor_command(const MotorCommand& cmd) override;
    bool get_flight_state(FlightState& state) override;

    bool set_target_position(double lat, double lon, float altitude) override;
    bool set_target_velocity(float vx, float vy, float vz) override;
    bool set_flight_mode(uint8_t mode) override;

    bool is_ready() override;
    float get_battery_percentage() override;

    bool takeoff(float altitude) override;
    bool land() override;
    bool goto_waypoint(const Waypoint& wp) override;

    void update() override;
    const char* get_mode_name() override { return "SIMULATE (Desktop)"; }

    // Get current position for map visualization
    FlightState get_current_state() const { return current_state; }

private:
    bool initialized;
    bool armed;
    uint8_t current_mode;

    FlightState current_state;
    FlightState target_state;
    MotorCommand current_command;

    double target_lat, target_lon;
    float target_alt;
    bool has_target;

    float battery_level;
    time_t last_update;

    // Physics simulation
    void simulate_physics(float dt);
    void apply_motor_forces(float dt);
    void apply_gravity(float dt);
    void apply_drag(float dt);
};

#endif // COMPILE_FOR_SIMULATE

