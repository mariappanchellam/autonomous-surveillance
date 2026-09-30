// mission_scheduler.h - Hourly scheduler for perimeter updates

#pragma once

#include "mission_planner.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <ctime>
#include <queue>

typedef std::function<bool(Coordinate[4])> PerimeterFetcher;

class MissionScheduler {
public:
    /**
     * Constructor with perimeter fetcher callback
     * fetcher: function that populates coordinates array and returns true on success
     */
    MissionScheduler(MissionPlanner* planner, PerimeterFetcher fetcher);

    ~MissionScheduler();

    /**
     * Start hourly scheduling
     * interval_seconds: update interval (default: 3600 = 1 hour)
     */
    bool start_scheduler(int interval_seconds = 3600);

    /**
     * Stop scheduler
     */
    void stop_scheduler();

    /**
     * Check if scheduler is running
     */
    bool is_running() const;

    /**
     * Get next scheduled update time
     */
    time_t get_next_update() const;

    /**
     * Force immediate update (for testing)
     */
    bool force_update();

    /**
     * Get update statistics
     */
    struct SchedulerStats {
        uint32_t total_updates;
        time_t last_update;
        time_t next_update;
        uint32_t successful_updates;
        uint32_t failed_updates;
    };
    SchedulerStats get_stats() const;

private:
    MissionPlanner* mission_planner;
    PerimeterFetcher perimeter_fetcher;

    std::thread scheduler_thread;
    mutable std::mutex scheduler_mutex;
    std::condition_variable scheduler_cv;

    bool running;
    int update_interval;
    time_t next_update_time;

    SchedulerStats stats;

    /**
     * Main scheduler loop (runs in separate thread)
     */
    void scheduler_loop();

    /**
     * Perform single update
     */
    bool perform_update();
};

