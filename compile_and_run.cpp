#include <iostream>
#include <array>
#include <utility>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <memory>
#include <fstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
#include <sstream>

// Platform detection
#if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS
    #include <windows.h>
    #include <tlhelp32.h>
    constexpr const char* OS = "windows";
#elif defined(__linux__) || defined(__unix__)
    #define PLATFORM_LINUX
    #include <unistd.h>
    #include <fstream>
    constexpr const char* OS = "linux";
#endif

//intellisense doesn't support modules as of march 2026
//import colorline; 
#include <colorline.hpp>
#include <terminal_utils.h> 

// ==================== Strategy Pattern Interface ====================
class PlatformStrategy {
public:
    const char pathSeparator = OS == "windows" ? '\\' : '/';

    virtual ~PlatformStrategy() = default;
    virtual std::string getParentProcessName() const = 0;
    virtual std::string getTerminalCommand(const std::string& runCmd) = 0;
    virtual std::string getCommandSeparator() const = 0;
    virtual std::string getGlobalConfigDir() const = 0;
};

// ==================== Windows Strategy ====================
#ifdef PLATFORM_WINDOWS
class WindowsStrategy : public PlatformStrategy {
private:
    std::string getParentProcessNameWindows() const {
        DWORD currentProcessId = GetCurrentProcessId();
        DWORD parentProcessId = 0;
        std::string parentProcessName = "unknown";

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            return parentProcessName;
        }

        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == currentProcessId) {
                    parentProcessId = pe32.th32ParentProcessID;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

        pe32.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == parentProcessId) {
                    parentProcessName = pe32.szExeFile;
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
        return parentProcessName;
    }

public:
    std::string getParentProcessName() const override {
        return getParentProcessNameWindows();
    }

    std::string getTerminalCommand(const std::string& runCmd) override {
        std::string parentProcessName = getParentProcessName();
        
        if (parentProcessName.find("pwsh") != std::string::npos || 
            parentProcessName.find("powershell") != std::string::npos
        ) {
            return "start powershell -NoExit -Command \"" + runCmd + "\"";
        } 
        else {
            return "start cmd /k \"" + runCmd + "\"";
        }
    }

    std::string getCommandSeparator() const override {
        std::string parentProcessName = getParentProcessName();
        return (parentProcessName.find("pwsh") != std::string::npos || 
                parentProcessName.find("powershell") != std::string::npos
        ) ? ";" : "&&";
    }

    std::string getGlobalConfigDir() const override {
        const char* appData = std::getenv("APPDATA");
        std::string baseDir = appData ? std::string(appData) : "C:\\";
        return baseDir + "\\compile_and_run";
    }
};
#endif

// ==================== Linux Strategy ====================
#ifdef PLATFORM_LINUX
class LinuxStrategy : public PlatformStrategy {
private:
    std::string getParentProcessNameLinux() const {
        pid_t parentPid = getppid();
        std::string path = "/proc/" + std::to_string(parentPid) + "/comm";
        std::ifstream commFile(path);
        std::string processName;
        
        if (commFile.is_open()) {
            std::getline(commFile, processName);
            commFile.close();
        }
        
        return processName.empty() ? "unknown" : processName;
    }

public:
    std::string getParentProcessName() const override {
        return getParentProcessNameLinux();
    }

    std::string getTerminalCommand(const std::string& runCmd) override {
        std::string parentProcessName = getParentProcessName();
        std::string terminal = "gnome-terminal";
        
        // Check for common terminals
        if (parentProcessName.find("gnome-terminal") != std::string::npos) {
            terminal = "gnome-terminal";
        } else if (parentProcessName.find("xterm") != std::string::npos) {
            terminal = "xterm";
        } else if (parentProcessName.find("konsole") != std::string::npos) {
            terminal = "konsole";
        } else if (parentProcessName.find("terminator") != std::string::npos) {
            terminal = "terminator";
        }
        
        // Escape quotes for shell command
        std::string escapedCmd = runCmd;
        size_t pos = 0;
        while ((pos = escapedCmd.find("\"", pos)) != std::string::npos) {
            escapedCmd.replace(pos, 1, "\\\"");
            pos += 2;
        }
        
        if (terminal == "gnome-terminal") {
            return terminal + " -- bash -c \"" + escapedCmd + "; exec bash\"";
        } else if (terminal == "konsole") {
            return terminal + " -e bash -c \"" + escapedCmd + "; exec bash\"";
        } else {
            return terminal + " -e bash -c \"" + escapedCmd + "; exec bash\"";
        }
    }

    std::string getCommandSeparator() const override {
        return ";";
    }

    std::string getGlobalConfigDir() const override {
        const char* home = std::getenv("HOME");
        std::string baseDir = home ? std::string(home) : "/tmp";
        return baseDir + "/.config/compile_and_run";
    }
};
#endif

class ConfigManager {
private:
    std::unordered_map<std::string, std::string> configs;

    // Sağdan ve soldan boşlukları temizleyen trim fonksiyonu (üşengeçlik yok!)
    std::string trim(const std::string& str) const {
        auto start = std::find_if_not(str.begin(), str.end(), [](unsigned char c) { return std::isspace(c); });
        auto end = std::find_if_not(str.rbegin(), str.rend(), [](unsigned char c) { return std::isspace(c); }).base();
        return (start < end) ? std::string(start, end) : "";
    }

    void createGlobalConfigInteractively(const std::string& path) {
        std::cout << "Global configuration file not found.\n";
        std::cout << "Creating new global config at: " << path << "\n\n";
        
        std::string compiler = TerminalReader::readWithGhostText("Default Compiler: ", "g++");
        if (compiler.empty()) compiler = "g++";

        std::string flags = TerminalReader::readWithGhostText("Default Flags: ", "-std=c++23 -Wall");
        if (flags.empty()) flags = "-std=c++23";

        std::string includes = TerminalReader::readWithGhostText("Default Includes (e.g., -Iinclude): ", "");
        std::string libs = TerminalReader::readWithGhostText("Default Libs (e.g., -lfmt -lpthread): ", "");

        std::vector<std::string> tools = {"none", "cmake", "make", "choose"};
        int toolIdx = TerminalReader::selectMenu("Build Tool Preference:", tools);
        std::string buildTool = tools[toolIdx];

        std::ofstream file(path);
        if (file.is_open()) {
            file << "# Global Configuration for compile_and_run\n";
            file << "COMPILER=" << compiler << "\n";
            file << "FLAGS=" << flags << "\n";
            file << "INCLUDES=" << includes << "\n";  
            file << "LIBS=" << libs << "\n";          
            file << "BUILD_TOOL=" << buildTool << "\n";
            file << "INCLUDES=\n";
            std::cout << "\nGlobal config created successfully!\n";
        } else {
            std::cerr << "CRITICAL ERROR: Could not create config file at " << path << "\n";
            std::cerr << "Please check folder permissions.\n";
            exit(EXIT_FAILURE);
        }
    }

    void loadFromFile(const std::string& path) {
        std::ifstream file(path);
        // is_open() false dönerse sorun yok (özellikle local config için)
        if (!file.is_open()) return;

        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;

            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));
                configs[key] = value;
            }
        }
    }

public:
    // localConfigPath default olarak boş string alıyor
    ConfigManager(
        const std::string& globalConfigDir, 
        const std::string& globalConfigPath, 
        const std::string& localConfigPath = ""
    ) {
        // Dizin yoksa oluştur
        if (!std::filesystem::exists(globalConfigDir)) {
            std::filesystem::create_directories(globalConfigDir);
        }

        // 1. Global dosya yoksa interaktif olarak oluştur
        if (!std::filesystem::exists(globalConfigPath)) {
            createGlobalConfigInteractively(globalConfigPath);
        }

        // 2. Global ayarları yükle
        loadFromFile(globalConfigPath);

        // 3. Local dosya yolu verilmişse (varsa) onu yükle ve ez
        if (!localConfigPath.empty()) {
            loadFromFile(localConfigPath);
        }

        // Her ihtimale karşı eksik kalan anahtarlar için güvenlik defaultları
        if (configs.find("COMPILER") == configs.end() || configs["COMPILER"].empty()) configs["COMPILER"] = "g++";
        if (configs.find("FLAGS") == configs.end()) configs["FLAGS"] = "-std=c++23";
        if (configs.find("BUILD_TOOL") == configs.end()) configs["BUILD_TOOL"] = "none";
    }

    std::string get(const std::string& key) const {
        auto it = configs.find(key);
        return it != configs.end() ? it->second : "";
    }
};

class CommandBuilder {
private:
    std::shared_ptr<PlatformStrategy> strategy;
    std::string fileName;
    std::string fileNameWithoutExt;
    std::string linkOptions;
    std::string currentDirName;
    std::string separator;
    ConfigManager& config;

public:
    CommandBuilder(std::shared_ptr<PlatformStrategy> strat, 
                   const std::string& file, 
                   const std::string& options,
                   const std::string& currentDir,
                   ConfigManager& conf
    ): strategy(strat), 
        fileName(file), 
        linkOptions(options), 
        currentDirName(currentDir), 
        config(conf) 
    {
        
        std::filesystem::path filePath(fileName);
        fileNameWithoutExt = filePath.stem().string();
        separator = strategy->getCommandSeparator();
    }

    std::string build() {
        std::string runCmd;
        const char sep = strategy->pathSeparator;
        std::string ext = OS == "windows" ? ".exe" : "";

        std::string compiler = config.get("COMPILER");
        std::string flags = config.get("FLAGS");
        std::string includes = config.get("INCLUDES");
        std::string libs = config.get("LIBS");

        std::string baseCompileCmd = compiler + " " + flags + " ";
        if (!includes.empty()) baseCompileCmd += includes + " ";
        baseCompileCmd += "\"" + fileName + "\" -o ";
        
        if (currentDirName == "src") {
            std::string outPath = "\".." + std::string(1, sep) + "bin" + sep + fileNameWithoutExt + ext + "\"";
            runCmd = baseCompileCmd + outPath;
            if (!libs.empty()) runCmd += " " + libs;
            if (!linkOptions.empty()) runCmd += " " + linkOptions;
            runCmd += " " + separator + " " + outPath;
        } else {
            std::string outPath = "\"" + fileNameWithoutExt + ext + "\"";
            runCmd = baseCompileCmd + outPath;
            if (!libs.empty()) runCmd += " " + libs;
            if (!linkOptions.empty()) runCmd += " " + linkOptions;
            runCmd += " " + separator + " " + outPath;
        }
        
        return runCmd;
    }
};

// ==================== Factory Method ====================
class PlatformFactory {
public:
    static std::shared_ptr<PlatformStrategy> createStrategy() {
        #ifdef PLATFORM_WINDOWS
            return std::make_shared<WindowsStrategy>();
        #elif defined(PLATFORM_LINUX)
            return std::make_shared<LinuxStrategy>();
        #else
            #error "Unsupported platform"
        #endif
    }
};

// ==================== Main Program ====================
int main(int argc, char* argv[]) {
    using enum Color;

    std::string fileName;
    // Check for command-line arguments
    if (argc >= 2) {
        fileName = argv[1];
    } else {
        fileName = TerminalReader::readWithGhostText("Enter the file name: ", "main.cpp");
    }

    // Create platform-specific strategy
    auto strategy = PlatformFactory::createStrategy();

    // Specify Global Config Path by OS
    std::string globalConfigDir = strategy->getGlobalConfigDir();
    std::string globalConfigPath = globalConfigDir + strategy->pathSeparator + "runner_config.txt";

    // Check Local Config Path (Give only the path if it exists, otherwise leave it blank)
    std::string currentDirPath = std::filesystem::current_path().string();
    std::string potentialLocalConfig = currentDirPath + strategy->pathSeparator + "runner_config.txt";
    std::string localConfigPath = std::filesystem::exists(potentialLocalConfig) ? potentialLocalConfig : "";

    ConfigManager config(globalConfigDir, globalConfigPath, localConfigPath);

    std::string buildTool = config.get("BUILD_TOOL");
    std::string runCmd;
    
    bool usingBuildSystem = false;

    if (buildTool == "cmake" && std::filesystem::exists("CMakeLists.txt")) {
        runCmd = "cmake -S . -B build && cmake --build build";
        usingBuildSystem = true;
    } 
    else if (buildTool == "make" && std::filesystem::exists("Makefile")) {
        runCmd = "make";
        usingBuildSystem = true;
    }
    // "choose" senaryosu (kullanıcıya sor)
    else if (buildTool == "choose") {
        std::vector<std::string> options = {
            "CMake (if CMakeLists.txt exists)",
            "Make (if Makefile exists)",
            "Standard Compile (g++/clang++)"
        };
        int choice = TerminalReader::selectMenu("What do you want to run?", options);
        
        if (choice == 0 && std::filesystem::exists("CMakeLists.txt")) {
            runCmd = "cmake -S . -B build && cmake --build build";
            usingBuildSystem = true;
        } else if (choice == 1 && std::filesystem::exists("Makefile")) {
            runCmd = "make";
            usingBuildSystem = true;
        }
    }

    // it will default to standard compile
    if (!usingBuildSystem) {
        std::string fileNameWithoutExt = fileName.substr(0, fileName.find_last_of("."));
        std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
        
        std::string prompt = "Enter link options for " + 
            Colored::text({Red, LightAqua}, fileNameWithoutExt, {LightGreen, LightGreen}) + 
            "." + Colored::text({LightAqua, Red}, ext) + ": ";
            
        std::string linkOptions = TerminalReader::readWithGhostText(prompt, "-static");

        std::string currentDirName = std::filesystem::current_path().filename().string();
        CommandBuilder builder(strategy, fileName, linkOptions, currentDirName, config);
        runCmd = builder.build();
    }

    // Get terminal command
    std::string terminalCmd = strategy->getTerminalCommand(runCmd);

    // Execute with colored output
    std::string parentProcessName = strategy->getParentProcessName();
    
    Colored::print("(" + parentProcessName + ") ", 0x4);
    Colored::print("Executing: ", 0xa);
    Colored::print(terminalCmd + "\n", 0xb);
    
    // Execute the command
    system(terminalCmd.c_str());

    return 0;
}