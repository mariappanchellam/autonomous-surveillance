// mission_scheduler.cpp - Implementation

#include "mission_scheduler.h"
#include <iostream>

MissionScheduler::MissionScheduler(MissionPlanner* planner, PerimeterFetcher fetcher)
    : mission_planner(planner), perimeter_fetcher(fetcher), running(false),
      update_interval(3600) {
    stats.total_updates = 0;
    stats.successful_updates = 0;
    stats.failed_updates = 0;
    stats.last_update = 0;
    stats.next_update = 0;
}

MissionScheduler::~MissionScheduler() {
    stop_scheduler();
}

bool MissionScheduler::start_scheduler(int interval_seconds) {
    if (running) return false;
    if (!mission_planner || !perimeter_fetcher) return false;

    update_interval = interval_seconds;
    running = true;
    next_update_time = time(nullptr) + interval_seconds;
    stats.next_update = next_update_time;

    scheduler_thread = std::thread(&MissionScheduler::scheduler_loop, this);
    return true;
}

void MissionScheduler::stop_scheduler() {
    if (!running) return;

    running = false;
    scheduler_cv.notify_one();

    if (scheduler_thread.joinable()) {
        scheduler_thread.join();
    }
}

bool MissionScheduler::is_running() const {
    return running;
}

time_t MissionScheduler::get_next_update() const {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    return next_update_time;
}

bool MissionScheduler::force_update() {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    return perform_update();
}

MissionScheduler::SchedulerStats MissionScheduler::get_stats() const {
    std::lock_guard<std::mutex> lock(scheduler_mutex);
    return stats;
}

void MissionScheduler::scheduler_loop() {
    std::unique_lock<std::mutex> lock(scheduler_mutex);

    while (running) {
        auto now = time(nullptr);
        auto wait_time = next_update_time - now;

        if (wait_time > 0) {
            // Wait until next update or stop signal
            scheduler_cv.wait_for(lock, std::chrono::seconds(wait_time));
        }

        if (!running) break;

        now = time(nullptr);
        if (now >= next_update_time) {
            perform_update();
            next_update_time = now + update_interval;
            stats.next_update = next_update_time;
        }
    }
}

bool MissionScheduler::perform_update() {
    if (!mission_planner) return false;

    stats.total_updates++;

    // Fetch new perimeter
    Coordinate corners[4];
    if (!perimeter_fetcher(corners)) {
        std::cerr << "Failed to fetch perimeter" << std::endl;
        stats.failed_updates++;
        return false;
    }

    // Check mission state
    MissionState state = mission_planner->get_state();

    // Only load new perimeter if idle or completed
    if (state == MissionState::IDLE || state == MissionState::COMPLETED) {
        if (!mission_planner->load_perimeter(corners)) {
            std::cerr << "Failed to load perimeter" << std::endl;
            stats.failed_updates++;
            return false;
        }

        if (!mission_planner->generate_mission()) {
            std::cerr << "Failed to generate mission" << std::endl;
            stats.failed_updates++;
            return false;
        }

        // Queue for execution if in REAL mode
        if (mission_planner->get_mode() == MissionMode::REAL) {
            mission_planner->queue_mission();
        }

        stats.successful_updates++;
        stats.last_update = time(nullptr);

        std::cout << "Mission updated at " << ctime(&stats.last_update);
        return true;
    } else if (state == MissionState::EXECUTING) {
        // Don't interrupt executing mission, just queue for later
        std::cout << "Mission in execution, queueing perimeter update..." << std::endl;

        // Could queue the perimeter for next execution
        mission_planner->queue_mission();

        stats.successful_updates++;
        return true;
    }

    stats.failed_updates++;
    return false;
}
