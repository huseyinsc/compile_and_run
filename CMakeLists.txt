cmake_minimum_required(VERSION 3.28)
project(CompileAndRun VERSION 1.0.0 LANGUAGES CXX)

# ==================== GLOBAL FetchContent Cache ====================
if(WIN32)
    set(DEFAULT_CACHE_DIR "$ENV{LOCALAPPDATA}/cpp-modules")
else()
    set(DEFAULT_CACHE_DIR "$ENV{HOME}/.cache/cpp-modules")
endif()

# Share the cache across all your projects (download once, use everywhere)
set(FETCHCONTENT_BASE_DIR "${DEFAULT_CACHE_DIR}" 
    CACHE PATH "Global shared cache for all C++ projects" FORCE)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_STATIC "Build static executable" OFF)

# ==================== Fetch Modules from GitHub ====================
include(FetchContent)

FetchContent_Declare(
    cpp_modules
    GIT_REPOSITORY https://github.com/huseyinsc/cpp-modules.git
    GIT_TAG        master          # İstersen "v1.0" tag atarsın, daha stabil olur
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

FetchContent_MakeAvailable(cpp_modules)

# Modülleri dahil et (her modül kendi CMakeLists.txt'ini çalıştırır)
add_subdirectory(
    ${cpp_modules_SOURCE_DIR}/terminal_utils
    ${cpp_modules_BINARY_DIR}/terminal_utils
)

add_subdirectory(
    ${cpp_modules_SOURCE_DIR}/colorline
    ${cpp_modules_BINARY_DIR}/colorline
)

# ==================== Main Executable ====================
add_executable(compile_and_run compile_and_run.cpp)

# Kütüphaneleri linkle (modüllerin PUBLIC include'ları otomatik gelir)
target_link_libraries(compile_and_run PRIVATE
    terminal_utils_lib
    colorline_lib
)

# ==================== Compiler & Platform Options ====================
if(WIN32)
    target_compile_definitions(compile_and_run PRIVATE PLATFORM_WINDOWS)
    target_link_libraries(compile_and_run PRIVATE)
elseif(UNIX)
    target_compile_definitions(compile_and_run PRIVATE PLATFORM_LINUX)
    target_link_libraries(compile_and_run PRIVATE pthread)
endif()

if(BUILD_STATIC)
    target_link_options(compile_and_run PRIVATE -static)
endif()

target_compile_options(compile_and_run PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/MP /permissive->
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
)

# ==================== Installation & Helper Targets ====================
option(INSTALL_TO_PATH "Install executable to user's PATH directory" OFF)

# Determine installation destination
if(INSTALL_TO_PATH)
    if(WIN32)
        # Windows: Install to %USERPROFILE%\bin
        set(INSTALL_BIN_DIR "$ENV{USERPROFILE}/bin")
    else()
        # Linux/macOS: Install to /usr/local/bin
        set(INSTALL_BIN_DIR "/usr/local/bin")
    endif()
    message(STATUS "CompileAndRun will be installed to: ${INSTALL_BIN_DIR}")
else()
    # Default: Install to project's bin folder (relative)
    set(INSTALL_BIN_DIR "bin")
endif()

install(TARGETS compile_and_run 
    RUNTIME DESTINATION ${INSTALL_BIN_DIR}
)

add_custom_target(config_summary
    COMMAND ${CMAKE_COMMAND} -E echo "============================================"
    COMMAND ${CMAKE_COMMAND} -E echo "CompileAndRun Configuration:"
    COMMAND ${CMAKE_COMMAND} -E echo "  C++ Standard : ${CMAKE_CXX_STANDARD}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Modules      : https://github.com/huseyinsc/cpp-modules"
    COMMAND ${CMAKE_COMMAND} -E echo "  Static       : ${BUILD_STATIC}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Install Path : ${INSTALL_BIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "  Generator    : ${CMAKE_GENERATOR}"
    COMMAND ${CMAKE_COMMAND} -E echo "============================================"
    COMMENT "Configuration summary"
)