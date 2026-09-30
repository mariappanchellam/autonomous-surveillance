# 🚁 ArduPilot Integration Guide

## Repository Structure

```
autonomous-surveillance/
├── (Autonomous Surveillance System - our code)
│   ├── mavlink_communicator.h/cpp
│   ├── surveillance_system.h/cpp
│   ├── audio_feedback.h/cpp
│   └── ... (other mission files)
│
└── ardupilot/  ← ArduPilot submodule (git submodule)
    ├── libraries/
    ├── ArduCopter/
    ├── Tools/
    └── waf (build script)
```

---

## Building with ArduPilot

### 1. Clone the Repository (with submodules)

```bash
git clone --recurse-submodules https://github.com/mariappanchellam/autonomous-surveillance.git
cd autonomous-surveillance

# Or if already cloned without submodules:
git submodule update --init --recursive
```

### 2. Initialize ArduPilot

```bash
cd ardupilot
pip install -U waf paramiko pyserial empy pexpect future
./waf --help
```

### 3. Build for Pixhawk 7+

```bash
# From ardupilot directory
./waf configure --board pixhawk7
./waf copter   # Build ArduCopter

# Binary will be in build/pixhawk7/bin/
# Upload with: ./Tools/autotest/sim_vehicle.py -v ArduCopter -f quad -l 37.7749,-122.4194,50,0
```

---

## Integrating Autonomous Surveillance

### Option A: Standalone Mission Module

Keep autonomous-surveillance separate, communicate via MAVLink:

```
PC Application (autonomous-surveillance)
    ↓ (MAVLink over USB/telemetry)
    ↓
Pixhawk 7+ (ArduCopter)
    ↓
Drone (physical flight)
```

**Advantages:**
- Clean separation of concerns
- Can update surveillance logic without recompiling firmware
- Works with standard ArduCopter

**Steps:**
1. Compile `mission_sim_app` or standalone mission planner
2. Build ArduCopter firmware normally
3. Upload firmware to Pixhawk
4. Run surveillance app on PC
5. Connect via USB serial port

### Option B: Firmware Integrated (Advanced)

Compile surveillance logic into ArduCopter firmware:

```bash
# Copy autonomous-surveillance headers to ArduPilot
cp *.h ardupilot/libraries/AP_Mission/
cp *.cpp ardupilot/libraries/AP_Mission/

# Modify ardupilot/libraries/AP_Mission/AP_Mission.h
# Add: #include "surveillance_system.h"

# Build
cd ardupilot
./waf configure --board pixhawk7
./waf copter

# Upload
./Tools/autotest/uploader.py ardupilot/build/pixhawk7/bin/ArduCopter
```

---

## Mission Workflow

### 1. Define Perimeter (4 GPS corners)

```cpp
Coordinate corners[] = {
    {37.7749, -122.4194},  // NW
    {37.7760, -122.4194},  // NE
    {37.7760, -122.4185},  // SE
    {37.7749, -122.4185}   // SW
};
```

### 2. Generate Mission

```cpp
WaypointGenerator gen;
std::vector<Waypoint> waypoints = 
    gen.generate_lawnmower(corners, 30.0, 50.0);  // 30m grid, 50m altitude
```

### 3. Upload to Pixhawk

```cpp
MAVLinkCommunicator comm;
comm.connect_to_pixhawk("/dev/ttyUSB0", 115200);
comm.upload_mission(waypoints);
comm.arm_drone();
comm.start_mission();
```

### 4. Surveillance Cycle

```cpp
SurveillanceSystem surv;
surv.initialize("/dev/ttyUSB0");
surv.set_master_point({37.7754, -122.4189, 50, "Master"});

// Auto: Fly → Land at Master → Wait 5min → Surveillance → Repeat hourly
surv.start_surveillance();
```

---

## Building for Different Boards

### Pixhawk 7+
```bash
./waf configure --board pixhawk7
./waf copter
```

### Pixhawk 4
```bash
./waf configure --board pixhawk4
./waf copter
```

### Pixhawk 1
```bash
./waf configure --board pixhawk1
./waf copter
```

### SITL (Software-in-the-Loop Simulation)
```bash
./waf configure --board sitl
./waf copter

# Run simulator
./build/sitl/bin/ArduCopter --home=-35.362881,149.165230,584,270
```

---

## Uploading Mission via USB

### Linux/Mac

```bash
cd autonomous-surveillance

# Build desktop app
mkdir build_real && cd build_real
cmake -DMISSION_MODE=REAL ..
make -j$(nproc)

# Run (connects to Pixhawk via USB)
./mission_upload_app
```

### Windows

1. Install CH340 drivers (for USB serial)
2. Build with: `cmake -G "Visual Studio 16 2019" -DMISSION_MODE=REAL ..`
3. Open .sln and build
4. Run executable

---

## Troubleshooting

### Connection Issues

```bash
# Check port
ls /dev/ttyUSB* /dev/ttyACM*

# Check baud rate (should be 115200)
stty -F /dev/ttyUSB0 115200

# Test connection
mavproxy.py --master=/dev/ttyUSB0:115200
```

### Firmware Upload Failed

```bash
# Update tool
pip install --upgrade pyserial

# Force upload
./Tools/uploader.py --port /dev/ttyUSB0 build/pixhawk7/bin/ArduCopter
```

### Mission Not Loading

- Check GPS lock (green LED on Pixhawk)
- Verify mission waypoints are valid
- Ensure mode is "Guided" or "Auto"
- Check battery level

---

## References

- ArduPilot Docs: https://ardupilot.org/copter/
- Pixhawk 7+ Hardware: https://ardupilot.org/copter/docs/common-pixhawk7.html
- MAVLink Protocol: https://mavlink.io/
- Mission Planner: http://ardupilot.org/planner/

---

## Next Steps

1. ✅ Clone repository with submodules
2. ✅ Build autonomous-surveillance (Option A - easiest)
3. ✅ Build ArduCopter firmware
4. ✅ Upload firmware to Pixhawk 7+
5. ✅ Run surveillance app
6. ✅ Upload mission via USB
7. ✅ Fly autonomous surveillance mission

---

**Status**: ✅ Ready for integration  
**Tested Hardware**: Pixhawk 7+  
**Tested Software**: ArduCopter Latest  
**Build System**: CMake + WAF  

