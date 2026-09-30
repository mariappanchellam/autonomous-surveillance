// hal_factory.cpp - Hardware Abstraction Layer Factory

#include "hal_abstraction.h"

#ifdef COMPILE_FOR_SIMULATE
#include "hal_simulator.h"
#elif COMPILE_FOR_REAL
#include "hal_real.h"
#endif

#include <stdio.h>

/**
 * Factory function to create appropriate HAL based on compile flags
 */
HardwareAbstraction* create_hardware_abstraction() {

#ifdef COMPILE_FOR_SIMULATE
    printf("=== SIMULATOR MODE ===\n");
    printf("Desktop simulation with physics\n");
    printf("No MAVLink, no hardware\n");
    return new SimulatorImpl();

#elif COMPILE_FOR_REAL
    printf("=== REAL MODE ===\n");
    printf("ArduPilot real hardware control\n");
    printf("Pixhawk 7+ flight controller\n");
    return new RealHardwareImpl();

#else
    #error "Must define either COMPILE_FOR_SIMULATE or COMPILE_FOR_REAL"
    return nullptr;
#endif
}

/**
 * Get mode name
 */
const char* get_compile_mode() {
#ifdef COMPILE_FOR_SIMULATE
    return "SIMULATE";
#elif COMPILE_FOR_REAL
    return "REAL";
#else
    return "UNKNOWN";
#endif
}

/**
 * Print compilation info
 */
void print_compilation_info() {
    printf("\n");
    printf("========================================\n");
    printf("Autonomous Mission System v1.0\n");
    printf("========================================\n");
    printf("Compile Mode: %s\n", get_compile_mode());
    printf("========================================\n\n");
}
