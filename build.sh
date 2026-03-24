#!/bin/bash
# build.sh - FetchContent ile modülleri otomatik çeken build script

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# ==================== HELP ====================
if [[ "$1" == "help" || "$1" == "--help" || "$1" == "-h" ]]; then
    echo -e "${GREEN}=== CompileAndRun Build Script Yardım ===${NC}"
    echo -e "${BLUE}Kullanım:${NC} ./build.sh [OPTIONS]"
    echo ""
    echo -e "${YELLOW}Mevcut parametreler:${NC}"
    echo -e "  ${GREEN}--clean${NC} | ${GREEN}-c${NC}          Build klasörünü temizler"
    echo -e "  ${GREEN}--debug${NC}               Debug modunda derler (Release varsayılan)"
    echo -e "  ${GREEN}--static${NC}              Statik executable oluşturur"
    echo -e "  ${GREEN}--purge-cache${NC}         GLOBAL cache’i tamamen siler (tehlikeli!)"    echo -e "  ${GREEN}--install${NC}             /usr/local/bin'e kurar (sudo gerektirebilir)"    echo -e "  ${GREEN}help${NC} | ${GREEN}--help${NC} | ${GREEN}-h${NC}   Bu yardım mesajını gösterir"
    echo ""
    echo -e "${YELLOW}Örnek kullanımlar:${NC}"
    echo -e "  ${BLUE}./build.sh${NC}                    → Normal Release build"
    echo -e "  ${BLUE}./build.sh --clean${NC}            → Build klasörünü temizleyip yeniden derle"
    echo -e "  ${BLUE}./build.sh --debug${NC}            → Debug modunda derle"
    echo -e "  ${BLUE}./build.sh --static${NC}           → Statik executable oluştur"
    echo -e "  ${BLUE}./build.sh --clean --debug${NC}    → Temizle + Debug derle"
    echo -e "  ${BLUE}./build.sh --purge-cache${NC}      → Cache’i sıfırla (yeniden indirecek)"    echo -e "  ${BLUE}./build.sh --install${NC}          → Build + /usr/local/bin'e yükle (sudo gerekebilir)"    echo ""
    echo -e "${GREEN}Cache konumu:${NC} ~/.cache/cpp-modules  (tüm projelerde ortak)"
    exit 0
fi

echo -e "${GREEN}=== CompileAndRun Build Başlıyor (Global Cache) ===${NC}"

GENERATOR="Ninja"

CLEAN=false
BUILD_TYPE="Release"
STATIC=false
PURGE_CACHE=false
INSTALL_TO_PATH=false

if [[ "$1" == "--clean" || "$1" == "-c" ]]; then CLEAN=true; fi
if [[ "$1" == "--debug" ]]; then BUILD_TYPE="Debug"; fi
if [[ "$1" == "--static" ]]; then STATIC=true; fi
if [[ "$1" == "--purge-cache" ]]; then PURGE_CACHE=true; fi
if [[ "$1" == "--install" ]]; then INSTALL_TO_PATH=true; fi

if [ "$CLEAN" = true ]; then
    echo -e "${YELLOW}Build klasörü temizleniyor...${NC}"
    rm -rf build
fi

if [ "$PURGE_CACHE" = true ]; then
    echo -e "${RED}!!! GLOBAL CACHE SİLİNİYOR !!!${NC}"
    rm -rf "$HOME/.cache/cpp-modules" 2>/dev/null || true
    echo -e "${YELLOW}Cache tamamen temizlendi. Sonraki build'de yeniden indirilecek.${NC}"
fi

echo -e "${GREEN}Building with ${BUILD_TYPE} (Static: ${STATIC})...${NC}"

cmake -B build -G "$GENERATOR" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DBUILD_STATIC="${STATIC}" \
    -DINSTALL_TO_PATH="${INSTALL_TO_PATH}"

cmake --build build --config "${BUILD_TYPE}"

if [ "$INSTALL_TO_PATH" = true ]; then
    echo -e "${YELLOW}/usr/local/bin'e kuruluyor...${NC}"
    sudo cmake --install build
    echo -e "${GREEN}compile_and_run başarıyla kuruldu!${NC}"
    echo -e "${BLUE}Kontrol et: ${NC}compile_and_run --help"
fi

echo -e "${GREEN}====================================${NC}"
echo -e "${GREEN}Build TAMAMLANDI!${NC}"
echo -e "Executable : ${GREEN}./build/compile_and_run${NC}"
echo -e "${YELLOW}Çalıştırmak için: ./build/compile_and_run${NC}"