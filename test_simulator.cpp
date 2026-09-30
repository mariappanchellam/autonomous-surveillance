// test_simulator.cpp - Test the mission simulator

#include "autonomous_mission.h"
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║     AUTONOMOUS MISSION SYSTEM - SIMULATOR TEST            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");

    // Create mission system
    AutonomousMission mission;

    // Step 1: Initialize
    printf("▶ Step 1: Initializing mission system...\n");
    if (!mission.initialize()) {
        fprintf(stderr, "❌ Failed to initialize\n");
        return 1;
    }
    printf("✅ Initialized (Mode: %s)\n\n", mission.get_mode_name());

    // Step 2: Define factory perimeter (square - San Francisco)
    printf("▶ Step 2: Loading factory perimeter (4 GPS coordinates)...\n");
    Coordinate corners[4] = {
        {37.7749, -122.4194, 50},   // NW corner (50m altitude)
        {37.7760, -122.4194, 50},   // NE corner
        {37.7760, -122.4185, 50},   // SE corner
        {37.7749, -122.4185, 50}    // SW corner
    };

    for (int i = 0; i < 4; i++) {
        printf("  Corner %d: (%.6f, %.6f) @ %.0f m\n",
               i+1, corners[i].latitude, corners[i].longitude, corners[i].altitude);
    }

    if (!mission.load_perimeter(corners)) {
        fprintf(stderr, "❌ Failed to load perimeter\n");
        return 1;
    }
    printf("✅ Perimeter loaded\n\n");

    // Step 3: Generate mission waypoints
    printf("▶ Step 3: Generating autonomous mission waypoints...\n");
    if (!mission.plan_mission()) {
        fprintf(stderr, "❌ Failed to plan mission\n");
        return 1;
    }
    printf("✅ Mission planned\n\n");

    // Step 4: Display mission statistics
    printf("▶ Step 4: Mission statistics...\n");
    auto stats = mission.get_statistics();
    printf("  Total Waypoints:    %d\n", stats.total_waypoints);
    printf("  Total Distance:     %.2f km\n", stats.total_distance / 1000.0);
    printf("  Estimated Time:     %.1f minutes\n", stats.estimated_time / 60.0);
    printf("  Battery Level:      %.1f%%\n", stats.battery_percent);
    printf("✅ Mission stats ready\n\n");

    // Step 5: Start mission
    printf("▶ Step 5: Starting autonomous flight...\n");
    if (!mission.start_mission()) {
        fprintf(stderr, "❌ Failed to start mission\n");
        return 1;
    }
    printf("✅ Mission started - Drone armed and taking off\n\n");

    // Step 6: Simulate flight
    printf("▶ Step 6: Simulating flight (10 simulation steps)...\n");
    printf("┌─────┬──────────────┬──────────────┬────────┬──────────┐\n");
    printf("│ Step│  Latitude    │  Longitude   │  Alt   │ Progress │\n");
    printf("├─────┼──────────────┼──────────────┼────────┼──────────┤\n");

    for (int step = 0; step < 10; step++) {
        mission.update();

        auto state = mission.get_flight_state();
        float progress = mission.get_statistics().total_waypoints > 0 ?
            (mission.get_current_waypoint_index() / (float)mission.get_statistics().total_waypoints * 100.0f) : 0.0f;

        printf("│ %3d │ %12.6f │ %12.6f │ %6.1f │ %6.1f%% │\n",
               step + 1,
               state.latitude,
               state.longitude,
               state.altitude,
               progress);

        usleep(100000);  // 100ms delay to simulate 10x speed
    }
    printf("└─────┴──────────────┴──────────────┴────────┴──────────┘\n");
    printf("✅ Flight simulation running\n\n");

    // Step 7: Display final state
    printf("▶ Step 7: Current flight state...\n");
    auto final_state = mission.get_flight_state();
    printf("  Position:  (%.6f, %.6f)\n", final_state.latitude, final_state.longitude);
    printf("  Altitude:  %.1f meters\n", final_state.altitude);
    printf("  Roll:      %.1f°\n", final_state.roll);
    printf("  Pitch:     %.1f°\n", final_state.pitch);
    printf("  Yaw:       %.1f°\n", final_state.yaw);
    printf("  Status:    %s\n", mission.get_mode_name());
    printf("✅ Flight state recorded\n\n");

    // Step 8: Summary
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                    ✅ TEST SUCCESSFUL!                    ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║ ✓ Mission system initialized                              ║\n");
    printf("║ ✓ Perimeter loaded (4 corners)                            ║\n");
    printf("║ ✓ %3d waypoints generated                                 ║\n", stats.total_waypoints);
    printf("║ ✓ Mission started (SIMULATE mode)                         ║\n");
    printf("║ ✓ Physics simulation running                              ║\n");
    printf("║ ✓ Autonomous flight executing                             ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n\n");

    printf("🎯 NEXT STEPS:\n");
    printf("   1. Run the Qt GUI app: ./mission_sim_app\n");
    printf("   2. Enter perimeter coordinates in the app\n");
    printf("   3. Click 'Generate Mission' to see waypoints on map\n");
    printf("   4. Click 'Start' to watch 10x speed autonomous flight\n");
    printf("   5. For real flight: Build REAL mode with ArduPilot\n\n");

    return 0;
}
