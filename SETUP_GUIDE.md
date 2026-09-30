# 🚀 Complete Setup Guide

## 📦 What You Have

Your GitHub repository contains:
- **Autonomous Surveillance System** - Mission planning, flight control, surveillance scheduling
- **ArduPilot Submodule** - Latest ArduCopter firmware (cloning in progress)

Repository: https://github.com/mariappanchellam/autonomous-surveillance

---

## 🔧 Prerequisites

### Linux/Mac

```bash
# Install dependencies
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git \
    qt6-base-dev qt6-declarative-dev \
    python3 python3-pip

# ArduPilot dependencies
pip install future ardupilot-requirements
```

### Windows

1. Install Git: https://git-scm.com/download/win
2. Install CMake: https://cmake.org/download/
3. Install Visual Studio 2019 or later
4. Install Qt6: https://www.qt.io/download
5. Install Python 3.8+

---

## 📥 Step 1: Clone Repository

### With ArduPilot Submodule (Full - 1GB+)

```bash
git clone --recurse-submodules https://github.com/mariappanchellam/autonomous-surveillance.git
cd autonomous-surveillance
```

### Without Submodule (Fast - 1MB)

```bash
git clone https://github.com/mariappanchellam/autonomous-surveillance.git
cd autonomous-surveillance
```

(You can add submodule later with: `git submodule update --init --recursive`)

---

## 🛠️ Step 2: Build SIMULATE Mode (Desktop Testing)

### Linux/Mac

```bash
mkdir build_sim
cd build_sim
cmake -DMISSION_MODE=SIMULATE ..
make -j$(nproc)
```

### Windows (Visual Studio)

```bash
mkdir build_sim
cd build_sim
cmake -G "Visual Studio 16 2019" -DMISSION_MODE=SIMULATE ..
cmake --build . --config Release
```

### Run Tests

```bash
# Test (no display needed)
./test_simulator

# GUI (needs display)
./mission_sim_app
```

---

## 🚁 Step 3: Build REAL Mode (Pixhawk 7+)

### Prerequisites

```bash
# Install ArduPilot build tools
pip install -U waf paramiko pyserial empy pexpect

# Clone ArduPilot if not using submodule
cd ardupilot
./waf --help
```

### Build for Pixhawk 7+

```bash
cd ardupilot

# Configure for Pixhawk 7+
./waf configure --board pixhawk7

# Build ArduCopter
./waf copter

# Binary location: build/pixhawk7/bin/ArduCopter
```

### Build for SITL (Simulation)

```bash
cd ardupilot

./waf configure --board sitl
./waf copter

# Run simulator
./build/sitl/bin/ArduCopter --home=-35.362881,149.165230,584,270
```

---

## 📤 Step 4: Upload Mission to Pixhawk 7+

### Method 1: Via USB (Recommended)

```bash
cd autonomous-surveillance/build_real

# Connect Pixhawk via USB
# The app will auto-detect the port

./mission_upload_app

# Steps:
# 1. Enter perimeter (4 GPS corners)
# 2. Click "Upload Mission"
# 3. Hear 5 beeps (confirmation)
# 4. Click "Start Flight"
```

### Method 2: Via Mission Planner

```bash
# Build desktop app
cd build_real
cmake -DMISSION_MODE=REAL ..
make -j$(nproc)

./mission_upload_app
```

---

## 🎯 Step 5: Deploy Autonomous Surveillance

### On your laptop/desktop with Pixhawk connected:

```bash
cd autonomous-surveillance

# Terminal 1: Start mission planner
./build_real/mission_upload_app

# Terminal 2: Monitor drone (optional)
mavproxy.py --master=/dev/ttyUSB0:115200
```

### Configuration:

1. **Perimeter** (4 GPS corners):
   - NW: 37.7749, -122.4194
   - NE: 37.7760, -122.4194
   - SE: 37.7760, -122.4185
   - SW: 37.7749, -122.4185

2. **Master Point** (landing site):
   - Latitude: 37.7754
   - Longitude: -122.4189
   - Altitude: 50m

3. **Surveillance Cycle**:
   - Initial wait: 5 minutes
   - Repeat interval: 60 minutes
   - Flight time: ~15 minutes per mission

---

## 🔌 Serial Port Configuration

### Find USB Port

#### Linux/Mac
```bash
ls /dev/ttyUSB*
ls /dev/ttyACM*
```

#### Windows
```cmd
mode
```

Look for COM3, COM4, etc.

### Set Permissions (Linux)

```bash
sudo chmod 666 /dev/ttyUSB0
```

Or add user to dialout group:

```bash
sudo usermod -a -G dialout $USER
```

---

## 🧪 Testing Workflow

### 1. Verify Compilation

```bash
cd autonomous-surveillance
./build_sim/test_simulator
```

Expected output:
```
✅ Mission Initialized
✅ Perimeter Loaded
✅ Waypoints Generated (14)
✅ Physics Simulation Running
✅ Autonomous Flight Executing
```

### 2. Test GUI (with display)

```bash
./build_sim/mission_sim_app
```

Expected:
- Red square (perimeter)
- Blue dots (waypoints)
- Green circle (drone position)
- Real-time animation

### 3. Test Pixhawk Connection

```bash
# With Pixhawk connected via USB
./build_real/mission_upload_app

# Should detect:
# - Serial port (COM3, /dev/ttyUSB0, etc.)
# - Pixhawk firmware version
- Battery level
```

---

## 🚀 Flying Your First Mission

### Pre-flight Checklist

- [ ] Pixhawk firmware uploaded
- [ ] GPS lock (green LED)
- [ ] Battery charged (3S LiPo minimum)
- [ ] Propellers installed correctly
- [ ] Mission uploaded
- [ ] Perimeter defined (4 corners)
- [ ] Master point set
- [ ] Compass calibrated
- [ ] Level calibration done

### Flight Sequence

1. Power on drone
2. Wait for GPS lock (green LED)
3. Connect via USB
4. Run mission app: `./mission_upload_app`
5. Click "Upload Mission"
6. Hear 5 beeps (confirmation)
7. Click "Start Flight"
8. Drone will:
   - Take off to 50m
   - Fly perimeter (lawnmower pattern)
   - Return to master point
   - Land
   - Wait 5 minutes
   - Repeat hourly

---

## 📊 Telemetry Monitoring

### Real-time Dashboard

```bash
# Mission app shows:
- Current position (lat/lon)
- Altitude
- Battery percentage
- Current waypoint
- Mission progress
- Flight time
```

### MAVProxy (Advanced)

```bash
mavproxy.py --master=/dev/ttyUSB0:115200 --sitl=127.0.0.1:5760

# Commands:
# status - Show drone status
# param list - Show all parameters
# wp list - Show mission waypoints
# arm - Arm motors
# disarm - Disarm motors
# mode auto - Set to autonomous mode
```

---

## 🐛 Troubleshooting

### "Serial port not found"

```bash
# Check connection
lsusb

# Check port
ls /dev/tty*

# Fix permissions (Linux)
sudo chmod 666 /dev/ttyUSB0
```

### "No GPS lock"

- Ensure GPS module is connected to Pixhawk
- Wait 30-60 seconds for lock
- Check green LED on Pixhawk
- Move outdoors away from buildings

### "Mission upload failed"

- Verify baud rate: 115200
- Check USB cable (try different cable)
- Reboot Pixhawk
- Verify mission waypoints are valid

### "Drone arms but won't take off"

- Check battery level (minimum 3S LiPo)
- Verify throttle in Mission Planner
- Check if in correct flight mode
- Verify accelerometer calibration

---

## 📚 Documentation

- `README.md` - Project overview
- `ARDUPILOT_INTEGRATION.md` - Detailed integration guide
- `API_DOCUMENTATION.md` - (Create as needed)
- `BUILD_AND_INTEGRATION.md` - Build instructions

---

## 🔗 Useful Links

- ArduPilot Docs: https://ardupilot.org
- Pixhawk 7+ Info: https://ardupilot.org/copter/docs/common-pixhawk7.html
- MAVLink Reference: https://mavlink.io
- Mission Planner: http://ardupilot.org/planner

---

## 📞 Support

For issues:
1. Check the GitHub issues: https://github.com/mariappanchellam/autonomous-surveillance/issues
2. Create detailed issue with:
   - Error message
   - Steps to reproduce
   - Hardware/software versions
   - Log files

---

## ✅ Summary

You now have:

1. ✅ Complete source code
2. ✅ Dual-mode build system (SIMULATE/REAL)
3. ✅ ArduPilot integration
4. ✅ Desktop testing tools
5. ✅ USB mission upload capability
6. ✅ Comprehensive documentation

**Next**: Follow the steps above to build and test!

---

**Last Updated**: September 30, 2026  
**Status**: ✅ Ready for deployment  
**Support**: https://github.com/mariappanchellam/autonomous-surveillance

