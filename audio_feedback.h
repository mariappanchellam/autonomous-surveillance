#pragma once

#include <string>
#include <cstdint>

class AudioFeedback {
public:
    AudioFeedback();
    ~AudioFeedback();

    enum AlertType {
        MISSION_LOADED = 0,    // 5 beeps
        TAKING_OFF = 1,        // 2 beeps
        ARRIVED_MASTER = 2,    // 3 beeps
        SURVEILLANCE_START = 3, // 4 beeps
        ERROR_ALERT = 4,       // Continuous beep
        BATTERY_LOW = 5        // Rapid beeps
    };

    // Play alert patterns
    bool beep(uint32_t count = 1, uint32_t duration_ms = 100, uint32_t interval_ms = 200);
    bool mission_loaded_alert();
    bool taking_off_alert();
    bool arrived_master_alert();
    bool surveillance_start_alert();
    bool error_alert();
    bool battery_low_alert();

    // Audio output control
    bool set_enabled(bool enabled);
    bool is_enabled() const;

    // System beeper control (hardware)
    bool beep_hardware(uint32_t frequency_hz = 1000, uint32_t duration_ms = 100);

private:
    bool enabled;
    bool use_hardware_beeper;

    bool play_alert_pattern(const std::string& pattern);
};
