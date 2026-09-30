#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <mutex>
#include <functional>
#include "mission_config.h"

class MAVLinkCommunicator {
public:
    MAVLinkCommunicator();
    ~MAVLinkCommunicator();

    // Connection management
    bool connect_to_pixhawk(const std::string& port, uint32_t baudrate = 115200);
    bool disconnect();
    bool is_connected() const;

    // Mission upload
    bool upload_mission(const std::vector<Waypoint>& waypoints);
    bool set_home_location(double latitude, double longitude, float altitude);

    // Flight control
    bool arm_drone();
    bool disarm_drone();
    bool takeoff(float altitude_m);
    bool land();
    bool start_mission();
    bool pause_mission();
    bool resume_mission();

    // Telemetry
    struct DroneState {
        double latitude;
        double longitude;
        float altitude;
        float yaw;
        float roll;
        float pitch;
        float battery_percent;
        uint8_t system_status;
        uint32_t mode;
    };

    bool get_drone_state(DroneState& state);

    // Callbacks
    using StateCallback = std::function<void(const DroneState&)>;
    void set_state_callback(StateCallback callback);

    // Message sending/receiving
    bool send_command(uint8_t command_id, const std::vector<uint8_t>& params);
    std::vector<uint8_t> receive_message(uint32_t timeout_ms = 1000);

private:
    int serial_port;
    bool connected;
    std::thread receive_thread;
    std::mutex state_mutex;
    DroneState current_state;
    StateCallback state_callback;

    void receive_loop();
    bool open_serial_port(const std::string& port, uint32_t baudrate);
    void close_serial_port();

    // MAVLink parsing
    bool parse_heartbeat(const std::vector<uint8_t>& data);
    bool parse_global_position_int(const std::vector<uint8_t>& data);
    bool parse_battery_status(const std::vector<uint8_t>& data);
};
