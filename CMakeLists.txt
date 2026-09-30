cmake_minimum_required(VERSION 3.16)
project(ArduPilotMissionDualMode CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ===================================
# Compile Mode Selection
# ===================================
# Set MISSION_MODE to either SIMULATE or REAL
# Default: SIMULATE
if(NOT MISSION_MODE)
    set(MISSION_MODE "SIMULATE")
endif()

message(STATUS "=================================")
message(STATUS "Autonomous Mission System")
message(STATUS "Compile Mode: ${MISSION_MODE}")
message(STATUS "=================================")

if(MISSION_MODE STREQUAL "SIMULATE")
    message(STATUS "Building for DESKTOP SIMULATOR")
    message(STATUS "No hardware required")
    message(STATUS "Physics simulation enabled")
    set(COMPILE_FLAG "COMPILE_FOR_SIMULATE")
    set(USE_QT6 TRUE)

elseif(MISSION_MODE STREQUAL "REAL")
    message(STATUS "Building for ARDUPILOT (Real Hardware)")
    message(STATUS "Target: Pixhawk 7+")
    message(STATUS "Warning: Requires ArduPilot source")
    set(COMPILE_FLAG "COMPILE_FOR_REAL")
    set(USE_QT6 FALSE)

else()
    message(FATAL_ERROR "MISSION_MODE must be SIMULATE or REAL")
endif()

# ===================================
# Core Mission Library (shared)
# ===================================
add_library(mission_core STATIC
    mission_config.h
    perimeter_handler.h
    perimeter_handler.cpp
    waypoint_generator.h
    waypoint_generator.cpp
    mission_planner.h
    mission_planner.cpp
    mission_scheduler.h
    mission_scheduler.cpp
)

target_include_directories(mission_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(mission_core PUBLIC ${COMPILE_FLAG})

# ===================================
# Hardware Abstraction Layer
# ===================================
if(MISSION_MODE STREQUAL "SIMULATE")
    add_library(hal STATIC
        hal_abstraction.h
        hal_simulator.h
        hal_simulator.cpp
        hal_factory.cpp
    )

elseif(MISSION_MODE STREQUAL "REAL")
    add_library(hal STATIC
        hal_abstraction.h
        hal_real.h
        hal_real.cpp
        hal_factory.cpp
    )

endif()

target_include_directories(hal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(hal PUBLIC ${COMPILE_FLAG})
target_link_libraries(hal PUBLIC mission_core)

# ===================================
# Autonomous Mission Module
# ===================================
add_library(autonomous_mission STATIC
    autonomous_mission.h
    autonomous_mission.cpp
)

target_include_directories(autonomous_mission PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(autonomous_mission PUBLIC ${COMPILE_FLAG})
target_link_libraries(autonomous_mission PUBLIC hal mission_core)

# ===================================
# Application (Desktop Simulator or ArduPilot)
# ===================================
if(MISSION_MODE STREQUAL "SIMULATE")
    # Desktop Simulator with Qt6 UI
    find_package(Qt6 COMPONENTS
        Core Gui Quick Qml
    REQUIRED)

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)

    add_library(qml_bridge STATIC
        qml_bridge.h
        qml_bridge.cpp
    )

    target_link_libraries(qml_bridge
        Qt6::Core Qt6::Gui Qt6::Quick Qt6::Qml
        autonomous_mission
    )

    target_compile_definitions(qml_bridge PUBLIC ${COMPILE_FLAG})

    # Main Application
    add_executable(mission_sim_app
        main.cpp
        mission.qrc
    )

    target_link_libraries(mission_sim_app
        Qt6::Gui Qt6::Quick
        qml_bridge autonomous_mission
    )

    # Copy QML files
    configure_file(main.qml main.qml COPYONLY)
    configure_file(MissionMapView.qml MissionMapView.qml COPYONLY)
    configure_file(MissionStatusPanel.qml MissionStatusPanel.qml COPYONLY)

elseif(MISSION_MODE STREQUAL "REAL")
    # ArduPilot Module - will be integrated into firmware
    # Create library for linking into ArduPilot
    # No standalone executable for REAL mode

    message(STATUS "")
    message(STATUS "ArduPilot Module Ready for Integration")
    message(STATUS "To integrate into ArduPilot:")
    message(STATUS "  1. Copy autonomous_mission library to ardupilot/libraries/")
    message(STATUS "  2. Update ardupilot/CMakeLists.txt to include this library")
    message(STATUS "  3. Modify AP_Mission.h to call autonomous mission APIs")
    message(STATUS "  4. Build with: ./waf configure --board pixhawk1")
    message(STATUS "                 ./waf build")
    message(STATUS "")

endif()

# ===================================
# Installation
# ===================================
if(MISSION_MODE STREQUAL "SIMULATE")
    install(TARGETS mission_sim_app DESTINATION bin)
    install(FILES main.qml MissionMapView.qml MissionStatusPanel.qml
            DESTINATION share/mission_sim)
endif()

# ===================================
# Build Info
# ===================================
message(STATUS "")
message(STATUS "Build Configuration:")
message(STATUS "  Mode: ${MISSION_MODE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "")
message(STATUS "To build:")
if(MISSION_MODE STREQUAL "SIMULATE")
    message(STATUS "  mkdir build && cd build")
    message(STATUS "  cmake -DMISSION_MODE=SIMULATE ..")
    message(STATUS "  make -j$(nproc)")
    message(STATUS "  ./mission_sim_app")
else()
    message(STATUS "  mkdir build && cd build")
    message(STATUS "  cmake -DMISSION_MODE=REAL -DARDUPILOTSRC=/path/to/ardupilot ..")
    message(STATUS "  make -j$(nproc)")
    message(STATUS "  (Then integrate into ArduPilot firmware build)")
endif()

# ===================================
# Test Program
# ===================================
add_executable(test_simulator
    test_simulator.cpp
)
target_link_libraries(test_simulator
    autonomous_mission
)

message(STATUS "")
