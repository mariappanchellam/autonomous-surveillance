# 🚁 Autonomous Surveillance System

**Real-time autonomous drone surveillance with ArduPilot Pixhawk 7+**

## Features

✅ **Mission Planning**
- Square perimeter definition (4 GPS corners)
- Automatic lawnmower grid waypoint generation
- Hourly perimeter updates

✅ **Real Flight Control**
- USB/COM port communication with Pixhawk 7+
- MAVLink protocol integration
- Mission upload and execution

✅ **Surveillance Automation**
- Fly to master point (nearest coordinates)
- 5 beeps on mission load confirmation
- Automatic surveillance cycle start (5 min after landing)
- Hourly scheduled surveillance flights
- Real-time telemetry monitoring

✅ **Desktop Simulator**
- Physics simulation (gravity, drag, motor forces)
- 10x real-time for testing
- Interactive map visualization
- No hardware required

---

## Quick Start

### Build SIMULATE Mode (Desktop Testing)
```bash
mkdir build_sim
cd build_sim
cmake -DMISSION_MODE=SIMULATE ..
make -j$(nproc)
./mission_sim_app
```

### Build REAL Mode (Pixhawk 7+)
```bash
mkdir build_real
cd build_real
cmake -DMISSION_MODE=REAL -DARDUPILOTSRC=/path/to/ardupilot ..
make -j$(nproc)
```

---

## Hardware Requirements (REAL Mode)

- **Pixhawk 7+** flight controller
- **USB/COM cable** for communication
- **Drone** (quadcopter or fixed-wing)
- **GPS module** for positioning
- **Telemetry radio** (optional, for remote monitoring)

---

## Mission Workflow

### 1. Define Perimeter
```
Corner 1: 37.7749, -122.4194 (NW)
Corner 2: 37.7760, -122.4194 (NE)
Corner 3: 37.7760, -122.4185 (SE)
Corner 4: 37.7749, -122.4185 (SW)
```

### 2. Generate Mission
- Automatic waypoint generation (lawnmower grid)
- 30m grid spacing
- 50m altitude

### 3. Load to Pixhawk
- Upload via USB/COM port
- Drone beeps 5 times (confirmation)
- Autonomous flight begins

### 4. Surveillance Cycle
- Fly to master point (land)
- Start surveillance after 5 minutes
- Repeat hourly automatically

---

## Project Structure

```
autonomous-surveillance/
├── CMakeLists.txt                 # Build configuration
├── mission_config.h               # Data structures
├── perimeter_handler.h/cpp        # Perimeter validation
├── waypoint_generator.h/cpp       # Waypoint generation
├── mission_planner.h/cpp          # Mission orchestration
├── mission_scheduler.h/cpp        # Hourly scheduling
│
├── hal_abstraction.h              # Hardware interface
├── hal_simulator.h/cpp            # Physics simulation
├── hal_real.h/cpp                 # ArduPilot integration
├── hal_factory.cpp                # Compile-time selector
│
├── autonomous_mission.h/cpp       # Mission controller
├── surveillance_system.h/cpp      # Surveillance logic
├── mavlink_communicator.h/cpp     # USB/COM communication
├── audio_feedback.h/cpp           # Beep control
│
├── qml_bridge.h/cpp               # Qt/C++ bridge
├── main.cpp                       # App entry point
├── main.qml                       # Main UI
├── MissionMapView.qml             # Map visualization
├── MissionStatusPanel.qml         # Statistics panel
├── mission.qrc                    # QML resources
│
└── test_simulator.cpp             # Test program
```

---

## Component Details

### Surveillance System
- Manages surveillance cycles
- Tracks master point coordinates
- Schedules hourly flights
- Monitors battery and telemetry

### MAVLink Communicator
- USB/Serial interface
- MAVLink protocol handling
- Waypoint upload
- Real-time telemetry streaming
- Status reporting

### Audio Feedback
- Beep on mission load (5 times)
- Audible alerts for system events
- Status confirmations

---

## Compilation Modes

### SIMULATE Mode (Desktop)
```bash
cmake -DMISSION_MODE=SIMULATE ..
# Desktop physics simulator
# Interactive QML GUI
# No hardware needed
```

### REAL Mode (Pixhawk)
```bash
cmake -DMISSION_MODE=REAL ..
# ArduPilot integration
# USB/COM communication
# Real flight control
```

---

## Testing

### Test Simulator (Headless)
```bash
./build_sim/test_simulator
```

### GUI Application (With Display)
```bash
./build_sim/mission_sim_app
```

---

## API Reference

See `API_DOCUMENTATION.md` for complete API details.

---

## License

MIT License - See LICENSE file

---

## Support

For issues, feature requests, or questions:
- Create an issue on GitHub
- Check documentation in `/docs` folder

---

**Status**: ✅ Ready for production deployment
**Latest Update**: September 30, 2026
