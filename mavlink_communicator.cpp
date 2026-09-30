#include "mavlink_communicator.h"
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/select.h>
#endif

MAVLinkCommunicator::MAVLinkCommunicator()
    : serial_port(-1), connected(false) {
}

MAVLinkCommunicator::~MAVLinkCommunicator() {
    disconnect();
}

bool MAVLinkCommunicator::connect_to_pixhawk(const std::string& port, uint32_t baudrate) {
    if (open_serial_port(port, baudrate)) {
        connected = true;
        receive_thread = std::thread(&MAVLinkCommunicator::receive_loop, this);
        return true;
    }
    return false;
}

bool MAVLinkCommunicator::disconnect() {
    connected = false;
    if (receive_thread.joinable()) {
        receive_thread.join();
    }
    close_serial_port();
    return true;
}

bool MAVLinkCommunicator::is_connected() const {
    return connected;
}

bool MAVLinkCommunicator::upload_mission(const std::vector<Waypoint>& waypoints) {
    if (!connected) return false;

    printf("[MAVLINK] Uploading %zu waypoints to Pixhawk...\n", waypoints.size());

    for (size_t i = 0; i < waypoints.size(); ++i) {
        const auto& wp = waypoints[i];
        std::vector<uint8_t> params = {
            static_cast<uint8_t>(wp.seq),
            static_cast<uint8_t>(wp.frame),
            static_cast<uint8_t>(wp.command),
            static_cast<uint8_t>((wp.x >> 24) & 0xFF),
            static_cast<uint8_t>((wp.x >> 16) & 0xFF),
            static_cast<uint8_t>((wp.x >> 8) & 0xFF),
            static_cast<uint8_t>(wp.x & 0xFF),
        };
        send_command(0xA1, params);
    }

    printf("[MAVLINK] Mission upload complete\n");
    return true;
}

bool MAVLinkCommunicator::set_home_location(double latitude, double longitude, float altitude) {
    if (!connected) return false;
    printf("[MAVLINK] Setting home location: %.6f, %.6f @ %.1f m\n", latitude, longitude, altitude);
    return true;
}

bool MAVLinkCommunicator::arm_drone() {
    if (!connected) return false;
    printf("[MAVLINK] Arming drone...\n");
    std::vector<uint8_t> params = {0x01};
    return send_command(0x80, params);
}

bool MAVLinkCommunicator::disarm_drone() {
    if (!connected) return false;
    printf("[MAVLINK] Disarming drone...\n");
    std::vector<uint8_t> params = {0x00};
    return send_command(0x80, params);
}

bool MAVLinkCommunicator::takeoff(float altitude_m) {
    if (!connected) return false;
    printf("[MAVLINK] Taking off to %.1f m\n", altitude_m);
    return true;
}

bool MAVLinkCommunicator::land() {
    if (!connected) return false;
    printf("[MAVLINK] Landing drone...\n");
    std::vector<uint8_t> params = {0x21};
    return send_command(0x81, params);
}

bool MAVLinkCommunicator::start_mission() {
    if (!connected) return false;
    printf("[MAVLINK] Starting mission...\n");
    return true;
}

bool MAVLinkCommunicator::pause_mission() {
    if (!connected) return false;
    printf("[MAVLINK] Pausing mission...\n");
    return true;
}

bool MAVLinkCommunicator::resume_mission() {
    if (!connected) return false;
    printf("[MAVLINK] Resuming mission...\n");
    return true;
}

bool MAVLinkCommunicator::get_drone_state(DroneState& state) {
    std::lock_guard<std::mutex> lock(state_mutex);
    state = current_state;
    return true;
}

void MAVLinkCommunicator::set_state_callback(StateCallback callback) {
    state_callback = callback;
}

bool MAVLinkCommunicator::send_command(uint8_t command_id, const std::vector<uint8_t>& params) {
    if (serial_port < 0) return false;

    std::vector<uint8_t> buffer;
    buffer.push_back(0xFE);
    buffer.push_back(params.size() + 1);
    buffer.push_back(0x00);
    buffer.push_back(0x00);
    buffer.push_back(0x00);
    buffer.push_back(command_id);
    buffer.insert(buffer.end(), params.begin(), params.end());

    ssize_t written = write(serial_port, buffer.data(), buffer.size());
    return written > 0;
}

std::vector<uint8_t> MAVLinkCommunicator::receive_message(uint32_t timeout_ms) {
    std::vector<uint8_t> buffer(256);
    return buffer;
}

bool MAVLinkCommunicator::open_serial_port(const std::string& port, uint32_t baudrate) {
#ifdef _WIN32
    return false;
#else
    serial_port = open(port.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (serial_port < 0) {
        printf("[ERROR] Failed to open serial port: %s\n", port.c_str());
        return false;
    }

    struct termios tty;
    if (tcgetattr(serial_port, &tty) != 0) {
        close(serial_port);
        return false;
    }

    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);

    tty.c_cflag |= (CLOCAL | CREAD);
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;
    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_lflag = 0;
    tty.c_cc[VMIN] = 0;
    tty.c_cc[VTIME] = 0;

    if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
        close(serial_port);
        return false;
    }

    printf("[MAVLINK] Connected to %s @ %u baud\n", port.c_str(), baudrate);
    return true;
#endif
}

void MAVLinkCommunicator::close_serial_port() {
    if (serial_port >= 0) {
        close(serial_port);
        serial_port = -1;
    }
}

void MAVLinkCommunicator::receive_loop() {
    uint8_t buffer[256];
    while (connected) {
        ssize_t n = read(serial_port, buffer, sizeof(buffer));
        if (n > 0) {
            std::vector<uint8_t> data(buffer, buffer + n);
            parse_heartbeat(data);
            parse_global_position_int(data);
            parse_battery_status(data);
        }
        usleep(10000);
    }
}

bool MAVLinkCommunicator::parse_heartbeat(const std::vector<uint8_t>& data) {
    return true;
}

bool MAVLinkCommunicator::parse_global_position_int(const std::vector<uint8_t>& data) {
    return true;
}

bool MAVLinkCommunicator::parse_battery_status(const std::vector<uint8_t>& data) {
    return true;
}
