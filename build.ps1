# build.ps1 - Windows build script that automatically fetches modules with FetchContent

param(
    [switch]$Clean,
    [switch]$Debug,
    [switch]$Static,
    [switch]$PurgeCache,
    [switch]$Install,
    [switch]$Help
)

# ==================== HELP ====================
if ($Help -or $args -contains "help" -or $args -contains "--help" -or $args -contains "-h") {
    Write-Host "=== CompileAndRun Build Script Help ===" -ForegroundColor Green
    Write-Host "Usage: .\build.ps1 [-Clean] [-Debug] [-Static] [-PurgeCache] [-Install] [-Help]" -ForegroundColor Blue
    Write-Host ""
    Write-Host "Available parameters:" -ForegroundColor Yellow
    Write-Host "  -Clean          : Clean the build directory" -ForegroundColor Green
    Write-Host "  -Debug          : Build in Debug mode (Release is default)" -ForegroundColor Green
    Write-Host "  -Static         : Create a static executable" -ForegroundColor Green
    Write-Host "  -PurgeCache     : Clear the GLOBAL cache completely (dangerous!)" -ForegroundColor Green    Write-Host "  -Install        : Install to %USERPROFILE%\bin" -ForegroundColor Green    Write-Host "  -Help           : Display this help message" -ForegroundColor Green
    Write-Host ""
    Write-Host "Example usages:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1                          → Normal Release build" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Clean                   → Clean build directory and rebuild" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Debug                   → Build in Debug mode" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Static                  → Create static executable" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Clean -Debug            → Clean + Debug build" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -PurgeCache              → Reset cache (will download again)" -ForegroundColor Blue    Write-Host "  .\build.ps1 -Install                 → Build + Install to %USERPROFILE%\bin" -ForegroundColor Blue    Write-Host ""
    Write-Host "Cache location: $env:LOCALAPPDATA\cpp-modules  (shared across all projects)" -ForegroundColor Green
    exit 0
}

Write-Host "=== CompileAndRun Build Starting (Global Cache) ===" -ForegroundColor Green

$BUILD_TYPE = if ($Debug) { "Debug" } else { "Release" }

if ($Clean) {
    Write-Host "Cleaning build directory..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue .\build
}

if ($PurgeCache) {
    Write-Host "!!! GLOBAL CACHE IS BEING DELETED !!!" -ForegroundColor Red
    $cachePath = if ($env:LOCALAPPDATA) { "$env:LOCALAPPDATA\cpp-modules" } else { "$env:USERPROFILE\.cache\cpp-modules" }
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $cachePath
    Write-Host "Cache completely cleared." -ForegroundColor Yellow
}

Write-Host "Building with $BUILD_TYPE (Static: $($Static.IsPresent))..." -ForegroundColor Green

cmake -B build -G Ninja `
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE `
    -DBUILD_STATIC=$($Static.IsPresent) `
    -DINSTALL_TO_PATH=$($Install.IsPresent)

cmake --build build --config $BUILD_TYPE

if ($Install) {
    Write-Host "Installing to $env:USERPROFILE\bin..." -ForegroundColor Yellow
    $binPath = "$env:USERPROFILE\bin"
    if (-not (Test-Path $binPath)) {
        New-Item -ItemType Directory -Path $binPath -Force | Out-Null
    }
    cmake --install build --config $BUILD_TYPE
    Write-Host "compile_and_run successfully installed!" -ForegroundColor Green
    Write-Host "Verify with: compile_and_run --help" -ForegroundColor Blue
}

Write-Host "====================================" -ForegroundColor Green
Write-Host "Build COMPLETED!" -ForegroundColor Green
Write-Host "Executable : .\build\compile_and_run.exe" -ForegroundColor Green
Write-Host "To run: .\build\compile_and_run.exe" -ForegroundColor Yellow