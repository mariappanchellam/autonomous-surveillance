#!/bin/bash

# Quick Start Script for Autonomous Surveillance System
# Usage: ./QUICK_START.sh [simulate|real|test]

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}🚁 Autonomous Surveillance System - Quick Start${NC}"
echo ""

# Default to simulate mode
MODE="${1:-simulate}"

case $MODE in
    simulate)
        echo -e "${GREEN}Building SIMULATE mode (Desktop testing)...${NC}"
        mkdir -p build_sim
        cd build_sim
        cmake -DMISSION_MODE=SIMULATE ..
        make -j$(nproc)
        echo ""
        echo -e "${GREEN}✅ Build complete!${NC}"
        echo ""
        echo "Run test program (headless):"
        echo "  ./test_simulator"
        echo ""
        echo "Run GUI application (needs display):"
        echo "  ./mission_sim_app"
        ;;

    real)
        echo -e "${GREEN}Building REAL mode (Pixhawk 7+)...${NC}"
        mkdir -p build_real
        cd build_real
        cmake -DMISSION_MODE=REAL ..
        make -j$(nproc)
        echo ""
        echo -e "${GREEN}✅ Build complete!${NC}"
        ;;

    test)
        echo -e "${GREEN}Running tests...${NC}"
        mkdir -p build_sim
        cd build_sim
        cmake -DMISSION_MODE=SIMULATE ..
        make -j$(nproc) test_simulator
        ./test_simulator
        ;;

    *)
        echo -e "${RED}Usage: $0 [simulate|real|test]${NC}"
        echo ""
        echo "  simulate  - Build desktop simulator with GUI"
        echo "  real      - Build for Pixhawk 7+ (requires ArduPilot)"
        echo "  test      - Run test program"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}✅ Done!${NC}"
