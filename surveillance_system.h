#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <chrono>
#include "mission_config.h"
#include "mavlink_communicator.h"
#include "audio_feedback.h"

class SurveillanceSystem {
public:
    SurveillanceSystem();
    ~SurveillanceSystem();

    struct MasterPoint {
        double latitude;
        double longitude;
        float altitude;
        std::string name;
    };

    enum SurveillanceState {
        IDLE = 0,
        INITIALIZING = 1,
        FLYING_TO_MASTER = 2,
        AT_MASTER = 3,
        WAITING_FOR_CYCLE = 4,
        SURVEILLANCE_ACTIVE = 5,
        RTL = 6,
        ERROR = 7
    };

    // Initialization
    bool initialize(const std::string& serial_port, uint32_t baudrate = 115200);
    bool shutdown();

    // Master point management
    bool set_master_point(const MasterPoint& point);
    bool get_master_point(MasterPoint& point) const;

    // Mission management
    bool upload_and_start_mission(const std::vector<Waypoint>& waypoints);

    // Surveillance control
    bool start_surveillance();
    bool stop_surveillance();
    bool pause_surveillance();

    // State queries
    SurveillanceState get_state() const;
    bool is_running() const;

    // Telemetry
    struct TelemetryData {
        double latitude;
        double longitude;
        float altitude;
        float battery_percent;
        uint32_t mission_time_sec;
        uint32_t total_distance_m;
        uint8_t waypoint_index;
        uint8_t total_waypoints;
    };

    bool get_telemetry(TelemetryData& data) const;

    // Callbacks
    using StateChangeCallback = std::function<void(SurveillanceState)>;
    using TelemetryCallback = std::function<void(const TelemetryData&)>;

    void set_state_callback(StateChangeCallback callback);
    void set_telemetry_callback(TelemetryCallback callback);

    // Settings
    void set_surveillance_interval_minutes(uint32_t minutes);
    void set_master_arrival_wait_minutes(uint32_t minutes);
    void set_battery_threshold_percent(float percent);

private:
    MAVLinkCommunicator mavlink;
    AudioFeedback audio;

    SurveillanceState current_state;
    MasterPoint master_point;

    std::thread surveillance_thread;
    std::thread telemetry_thread;
    bool running;

    std::mutex state_mutex;
    std::mutex telemetry_mutex;

    TelemetryData telemetry;

    StateChangeCallback state_callback;
    TelemetryCallback telemetry_callback;

    uint32_t surveillance_interval_min;
    uint32_t master_arrival_wait_min;
    float battery_threshold;

    void surveillance_loop();
    void telemetry_loop();

    void change_state(SurveillanceState new_state);
    void update_telemetry();

    bool fly_to_master_point();
    bool wait_at_master_point();
    bool start_surveillance_cycle();
    bool land_at_master();
};
