#!/bin/bash
# build.sh - FetchContent ile modülleri otomatik çeken build script

set -e  # Hata olursa hemen çıksın

# Renkler (eski hali gibi)
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${GREEN}=== CompileAndRun Build Başlıyor (FetchContent modules) ===${NC}"

# Her zaman Ninja (en hızlı + C++ module desteği mükemmel)
GENERATOR="Ninja"

# Opsiyonel parametreler
CLEAN=false
BUILD_TYPE="Release"
STATIC=false

if [[ "$1" == "--clean" || "$1" == "-c" ]]; then
    CLEAN=true
fi
if [[ "$1" == "--debug" || "$2" == "--debug" ]]; then
    BUILD_TYPE="Debug"
fi
if [[ "$1" == "--static" || "$2" == "--static" ]]; then
    STATIC=true
fi

if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Build klasörü temizleniyor...${NC}"
    rm -rf build
fi

echo -e "${GREEN}Building with ${BUILD_TYPE} (Static: ${STATIC})...${NC}"

cmake -B build -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_STATIC="${STATIC}"

cmake --build build --config "${BUILD_TYPE}"

echo -e "${GREEN}====================================${NC}"
echo -e "${GREEN}Build TAMAMLANDI!${NC}"
echo -e "Executable: ${GREEN}./build/compile_and_run${NC}"
echo -e "${YELLOW}Çalıştırmak için: ./build/compile_and_run${NC}"