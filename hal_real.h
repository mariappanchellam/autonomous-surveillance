// hal_real.h - Real Hardware Implementation (ArduPilot target)

#pragma once

#include "hal_abstraction.h"

#ifdef COMPILE_FOR_REAL

// ArduPilot includes (when compiling for Pixhawk/real hardware)
// #include "AP_AHRS/AP_AHRS.h"
// #include "AP_Motors/AP_Motors.h"
// #include "AP_GPS/AP_GPS.h"
// etc.

class RealHardwareImpl : public HardwareAbstraction {
public:
    RealHardwareImpl();
    ~RealHardwareImpl();

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
    const char* get_mode_name() override { return "REAL (ArduPilot)"; }

private:
    // ArduPilot object pointers
    // AP_Motors* motors;
    // AP_AHRS* ahrs;
    // AP_GPS* gps;
    // etc.

    bool initialized;
    bool armed;
    uint8_t current_mode;

    // Convert waypoint to motor commands
    void waypoint_to_motor_cmd(const Waypoint& wp, MotorCommand& cmd);
};

#endif // COMPILE_FOR_REAL

