#!/bin/bash
# build.sh - Build script that automatically fetches modules with FetchContent

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# ==================== HELP ====================
show_help() {
    echo -e "${GREEN}=== CompileAndRun Build Script Help ===${NC}"
    echo -e "${BLUE}Usage:${NC} ./build.sh [OPTIONS]"
    echo ""
    echo -e "${YELLOW}Available parameters:${NC}"
    echo -e "  ${GREEN}--clean${NC} | ${GREEN}-c${NC}          Clean the build directory"
    echo -e "  ${GREEN}--debug${NC}               Build in Debug mode (Release is default)"
    echo -e "  ${GREEN}--static${NC}              Create a static executable"
    echo -e "  ${GREEN}--purge-cache${NC}         Clear the GLOBAL cache completely (dangerous!)"
    echo -e "  ${GREEN}--install${NC}             Install to /usr/local/bin (may require sudo)"
    echo -e "  ${GREEN}help${NC} | ${GREEN}--help${NC} | ${GREEN}-h${NC}   Display this help message"
    echo ""
    echo -e "${YELLOW}Example usages:${NC}"
    echo -e "  ${BLUE}./build.sh${NC}                          → Normal Release build"
    echo -e "  ${BLUE}./build.sh --clean${NC}                  → Clean build directory and rebuild"
    echo -e "  ${BLUE}./build.sh --debug${NC}                  → Build in Debug mode"
    echo -e "  ${BLUE}./build.sh --static${NC}                 → Create static executable"
    echo -e "  ${BLUE}./build.sh --clean --debug${NC}          → Clean + Debug build"
    echo -e "  ${BLUE}./build.sh --purge-cache${NC}            → Reset cache (will download again)"
    echo -e "  ${BLUE}./build.sh --install${NC}                → Build + Install to /usr/local/bin (may require sudo)"
    echo -e "  ${BLUE}./build.sh --clean --static --install${NC} → Clean + Static + Install"
    echo ""
    echo -e "${GREEN}Cache location:${NC} ~/.cache/cpp-modules  (shared across all projects)"
}

if [[ "$1" == "help" || "$1" == "--help" || "$1" == "-h" ]]; then
    show_help
    exit 0
fi

echo -e "${GREEN}=== CompileAndRun Build Starting (Global Cache) ===${NC}"

GENERATOR="Ninja"

CLEAN=false
BUILD_TYPE="Release"
STATIC=false
PURGE_CACHE=false
INSTALL_TO_PATH=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean|-c)
            CLEAN=true
            ;;
        --debug)
            BUILD_TYPE="Debug"
            ;;
        --static)
            STATIC=true
            ;;
        --purge-cache)
            PURGE_CACHE=true
            ;;
        --install)
            INSTALL_TO_PATH=true
            ;;
        --help|-h|help)
            show_help
            exit 0
            ;;
        *)
            echo -e "${RED}Bilinmeyen parametre: $1${NC}"
            echo -e "${YELLOW}Yardım için: ./build.sh --help${NC}"
            exit 1
            ;;
    esac
    shift
done

if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build
fi

if [ "$PURGE_CACHE" = true ]; then
    echo -e "${RED}!!! GLOBAL CACHE IS BEING DELETED !!!${NC}"
    rm -rf "$HOME/.cache/cpp-modules" 2>/dev/null || true
    echo -e "${YELLOW}Cache completely cleared. Will download again on next build.${NC}"
fi

echo -e "${GREEN}Building with ${BUILD_TYPE} (Static: ${STATIC})...${NC}"

cmake -B build -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_STATIC="${STATIC}" \
    -DINSTALL_TO_PATH="${INSTALL_TO_PATH}"

cmake --build build --config "${BUILD_TYPE}"

if [ "$INSTALL_TO_PATH" = true ]; then
    echo -e "${YELLOW}Installing to /usr/local/bin...${NC}"
    sudo cmake --install build
    echo -e "${GREEN}compile_and_run successfully installed!${NC}"
    echo -e "${BLUE}Verify with: ${NC}compile_and_run --help"
fi

echo -e "${GREEN}====================================${NC}"
echo -e "${GREEN}Build COMPLETED!${NC}"
echo -e "Executable : ${GREEN}./build/compile_and_run${NC}"
echo -e "${YELLOW}To run: ./build/compile_and_run${NC}"