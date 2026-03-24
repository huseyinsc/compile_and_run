# build.ps1 - FetchContent ile modülleri otomatik çeken Windows build script

param(
    [switch]$Clean,
    [switch]$Debug,
    [switch]$Static,
    [switch]$PurgeCache,
    [switch]$Help
)

# ==================== HELP ====================
if ($Help -or $args -contains "help" -or $args -contains "--help" -or $args -contains "-h") {
    Write-Host "=== CompileAndRun Build Script Yardım ===" -ForegroundColor Green
    Write-Host "Kullanım: .\build.ps1 [-Clean] [-Debug] [-Static] [-PurgeCache] [-Help]" -ForegroundColor Blue
    Write-Host ""
    Write-Host "Mevcut parametreler:" -ForegroundColor Yellow
    Write-Host "  -Clean          : Build klasörünü temizler" -ForegroundColor Green
    Write-Host "  -Debug          : Debug modunda derler (varsayılan Release)" -ForegroundColor Green
    Write-Host "  -Static         : Statik executable oluşturur" -ForegroundColor Green
    Write-Host "  -PurgeCache     : GLOBAL cache’i tamamen siler (tehlikeli!)" -ForegroundColor Green
    Write-Host "  -Help           : Bu yardım mesajını gösterir" -ForegroundColor Green
    Write-Host ""
    Write-Host "Örnek kullanımlar:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1                          → Normal Release build" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Clean                   → Build klasörünü temizleyip yeniden derle" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Debug                   → Debug modunda derle" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Static                  → Statik executable oluştur" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -Clean -Debug            → Temizle + Debug derle" -ForegroundColor Blue
    Write-Host "  .\build.ps1 -PurgeCache              → Cache’i sıfırla (yeniden indirecek)" -ForegroundColor Blue
    Write-Host ""
    Write-Host "Cache konumu: $env:LOCALAPPDATA\cpp-modules  (tüm projelerde ortak)" -ForegroundColor Green
    exit 0
}

Write-Host "=== CompileAndRun Build Başlıyor (Global Cache) ===" -ForegroundColor Green

$BUILD_TYPE = if ($Debug) { "Debug" } else { "Release" }

if ($Clean) {
    Write-Host "Build klasörü temizleniyor..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue .\build
}

if ($PurgeCache) {
    Write-Host "!!! GLOBAL CACHE SİLİNİYOR !!!" -ForegroundColor Red
    $cachePath = if ($env:LOCALAPPDATA) { "$env:LOCALAPPDATA\cpp-modules" } else { "$env:USERPROFILE\.cache\cpp-modules" }
    Remove-Item -Recurse -Force -ErrorAction SilentlyContinue $cachePath
    Write-Host "Cache tamamen temizlendi." -ForegroundColor Yellow
}

Write-Host "Building with $BUILD_TYPE (Static: $($Static.IsPresent))..." -ForegroundColor Green

cmake -B build -G Ninja `
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE `
    -DBUILD_STATIC=$($Static.IsPresent)

cmake --build build --config $BUILD_TYPE

Write-Host "====================================" -ForegroundColor Green
Write-Host "Build TAMAMLANDI!" -ForegroundColor Green
Write-Host "Executable : .\build\compile_and_run.exe" -ForegroundColor Green
Write-Host "Çalıştırmak için: .\build\compile_and_run.exe" -ForegroundColor Yellow