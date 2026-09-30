#include "audio_feedback.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <chrono>
#include <thread>
#include <cstring>
#include <iostream>

#ifdef __linux__
    #include <linux/input.h>
#endif

AudioFeedback::AudioFeedback()
    : enabled(true), use_hardware_beeper(true) {
}

AudioFeedback::~AudioFeedback() {
}

bool AudioFeedback::beep(uint32_t count, uint32_t duration_ms, uint32_t interval_ms) {
    if (!enabled) return false;

    for (uint32_t i = 0; i < count; ++i) {
        if (i > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }
        beep_hardware(1000, duration_ms);
    }
    return true;
}

bool AudioFeedback::mission_loaded_alert() {
    // 5 beeps - confirmation
    printf("[AUDIO] Mission loaded alert (5 beeps)\n");
    return beep(5, 100, 150);
}

bool AudioFeedback::taking_off_alert() {
    // 2 beeps - takeoff
    printf("[AUDIO] Taking off alert (2 beeps)\n");
    return beep(2, 150, 200);
}

bool AudioFeedback::arrived_master_alert() {
    // 3 beeps - arrival
    printf("[AUDIO] Arrived at master point (3 beeps)\n");
    return beep(3, 200, 250);
}

bool AudioFeedback::surveillance_start_alert() {
    // 4 beeps - surveillance start
    printf("[AUDIO] Surveillance cycle started (4 beeps)\n");
    return beep(4, 150, 150);
}

bool AudioFeedback::error_alert() {
    // Continuous beep
    printf("[AUDIO] Error alert (continuous)\n");
    return beep_hardware(800, 500);
}

bool AudioFeedback::battery_low_alert() {
    // Rapid beeps
    printf("[AUDIO] Battery low alert (rapid beeps)\n");
    return beep(10, 50, 50);
}

bool AudioFeedback::set_enabled(bool enabled) {
    this->enabled = enabled;
    return true;
}

bool AudioFeedback::is_enabled() const {
    return enabled;
}

bool AudioFeedback::beep_hardware(uint32_t frequency_hz, uint32_t duration_ms) {
    if (!enabled) return false;

#ifdef __linux__
    // Try to access /dev/tty0 for system beeper
    int fd = open("/dev/tty0", O_WRONLY);
    if (fd >= 0) {
        printf("\x07");
        fflush(stdout);
        close(fd);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        return true;
    }
#endif

    // Fallback: just print a beep character
    printf("\x07");
    fflush(stdout);
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
    return true;
}

bool AudioFeedback::play_alert_pattern(const std::string& pattern) {
    return true;
}
