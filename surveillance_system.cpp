#include "surveillance_system.h"
#include <chrono>
#include <iostream>
#include <cmath>

SurveillanceSystem::SurveillanceSystem()
    : current_state(IDLE), running(false),
      surveillance_interval_min(60),
      master_arrival_wait_min(5),
      battery_threshold(20.0f) {
    telemetry = {0, 0, 0, 100.0f, 0, 0, 0, 0};
}

SurveillanceSystem::~SurveillanceSystem() {
    shutdown();
}

bool SurveillanceSystem::initialize(const std::string& serial_port, uint32_t baudrate) {
    if (!mavlink.connect_to_pixhawk(serial_port, baudrate)) {
        printf("[SURVEILLANCE] Failed to connect to Pixhawk\n");
        return false;
    }

    running = true;
    change_state(INITIALIZING);

    surveillance_thread = std::thread(&SurveillanceSystem::surveillance_loop, this);
    telemetry_thread = std::thread(&SurveillanceSystem::telemetry_loop, this);

    printf("[SURVEILLANCE] System initialized successfully\n");
    return true;
}

bool SurveillanceSystem::shutdown() {
    running = false;

    if (surveillance_thread.joinable()) {
        surveillance_thread.join();
    }
    if (telemetry_thread.joinable()) {
        telemetry_thread.join();
    }

    mavlink.disconnect();
    change_state(IDLE);

    printf("[SURVEILLANCE] System shutdown\n");
    return true;
}

bool SurveillanceSystem::set_master_point(const MasterPoint& point) {
    std::lock_guard<std::mutex> lock(state_mutex);
    master_point = point;
    printf("[SURVEILLANCE] Master point set: %.6f, %.6f @ %.1f m (%s)\n",
           point.latitude, point.longitude, point.altitude, point.name.c_str());
    return true;
}

bool SurveillanceSystem::get_master_point(MasterPoint& point) const {
    std::lock_guard<std::mutex> lock(state_mutex);
    point = master_point;
    return true;
}

bool SurveillanceSystem::upload_and_start_mission(const std::vector<Waypoint>& waypoints) {
    printf("[SURVEILLANCE] Uploading mission with %zu waypoints\n", waypoints.size());

    if (!mavlink.upload_mission(waypoints)) {
        printf("[SURVEILLANCE] Mission upload failed\n");
        return false;
    }

    // Play 5 beeps to confirm mission loaded
    audio.mission_loaded_alert();

    if (!mavlink.start_mission()) {
        printf("[SURVEILLANCE] Mission start failed\n");
        return false;
    }

    audio.taking_off_alert();
    printf("[SURVEILLANCE] Mission started\n");
    return true;
}

bool SurveillanceSystem::start_surveillance() {
    if (current_state != IDLE) {
        printf("[SURVEILLANCE] Cannot start - system not idle\n");
        return false;
    }

    change_state(FLYING_TO_MASTER);
    return true;
}

bool SurveillanceSystem::stop_surveillance() {
    mavlink.land();
    change_state(IDLE);
    return true;
}

bool SurveillanceSystem::pause_surveillance() {
    mavlink.pause_mission();
    return true;
}

SurveillanceSystem::SurveillanceState SurveillanceSystem::get_state() const {
    std::lock_guard<std::mutex> lock(state_mutex);
    return current_state;
}

bool SurveillanceSystem::is_running() const {
    return running && current_state != IDLE;
}

bool SurveillanceSystem::get_telemetry(TelemetryData& data) const {
    std::lock_guard<std::mutex> lock(telemetry_mutex);
    data = telemetry;
    return true;
}

void SurveillanceSystem::set_state_callback(StateChangeCallback callback) {
    state_callback = callback;
}

void SurveillanceSystem::set_telemetry_callback(TelemetryCallback callback) {
    telemetry_callback = callback;
}

void SurveillanceSystem::set_surveillance_interval_minutes(uint32_t minutes) {
    surveillance_interval_min = minutes;
    printf("[SURVEILLANCE] Surveillance interval set to %u minutes\n", minutes);
}

void SurveillanceSystem::set_master_arrival_wait_minutes(uint32_t minutes) {
    master_arrival_wait_min = minutes;
    printf("[SURVEILLANCE] Master point wait time set to %u minutes\n", minutes);
}

void SurveillanceSystem::set_battery_threshold_percent(float percent) {
    battery_threshold = percent;
}

void SurveillanceSystem::surveillance_loop() {
    auto cycle_start = std::chrono::steady_clock::now();

    while (running) {
        std::lock_guard<std::mutex> lock(state_mutex);

        switch (current_state) {
            case FLYING_TO_MASTER:
                printf("[SURVEILLANCE] Flying to master point\n");
                fly_to_master_point();
                change_state(WAITING_FOR_CYCLE);
                break;

            case WAITING_FOR_CYCLE:
                printf("[SURVEILLANCE] Waiting at master point (5 min)\n");
                wait_at_master_point();
                change_state(SURVEILLANCE_ACTIVE);
                break;

            case SURVEILLANCE_ACTIVE:
                printf("[SURVEILLANCE] Starting surveillance cycle\n");
                audio.surveillance_start_alert();
                start_surveillance_cycle();
                change_state(FLYING_TO_MASTER);
                break;

            default:
                break;
        }

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void SurveillanceSystem::telemetry_loop() {
    while (running) {
        update_telemetry();

        if (telemetry_callback) {
            telemetry_callback(telemetry);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void SurveillanceSystem::change_state(SurveillanceState new_state) {
    if (current_state != new_state) {
        current_state = new_state;
        printf("[SURVEILLANCE] State changed to: %d\n", new_state);

        if (state_callback) {
            state_callback(new_state);
        }
    }
}

void SurveillanceSystem::update_telemetry() {
    std::lock_guard<std::mutex> lock(telemetry_mutex);

    MAVLinkCommunicator::DroneState drone_state;
    if (mavlink.get_drone_state(drone_state)) {
        telemetry.latitude = drone_state.latitude;
        telemetry.longitude = drone_state.longitude;
        telemetry.altitude = drone_state.altitude;
        telemetry.battery_percent = drone_state.battery_percent;
    }
}

bool SurveillanceSystem::fly_to_master_point() {
    audio.taking_off_alert();
    mavlink.arm_drone();
    mavlink.takeoff(master_point.altitude);

    std::this_thread::sleep_for(std::chrono::seconds(10));

    audio.arrived_master_alert();
    return true;
}

bool SurveillanceSystem::wait_at_master_point() {
    printf("[SURVEILLANCE] Landing at master point\n");
    mavlink.land();

    // Wait 5 minutes before starting surveillance
    std::this_thread::sleep_for(
        std::chrono::minutes(master_arrival_wait_min)
    );

    return true;
}

bool SurveillanceSystem::start_surveillance_cycle() {
    printf("[SURVEILLANCE] Surveillance cycle running (duration: %u min)\n",
           surveillance_interval_min);

    // In real implementation, this would fly perimeter pattern
    std::this_thread::sleep_for(
        std::chrono::minutes(surveillance_interval_min)
    );

    return true;
}

bool SurveillanceSystem::land_at_master() {
    printf("[SURVEILLANCE] Landing at master point\n");
    mavlink.land();
    return true;
}
