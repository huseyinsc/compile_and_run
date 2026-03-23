#include <iostream>
#include <array>
#include <utility>
#include <string>
#include <filesystem>
#include <cstdlib>
#include <memory>

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

// ==================== Strategy Pattern Interface ====================
class PlatformStrategy {
public:
    const char pathSeparator = OS == "windows" ? '\\' : '/';

    virtual ~PlatformStrategy() = default;
    virtual std::string getParentProcessName() const = 0;
    virtual std::string getTerminalCommand(const std::string& runCmd) = 0;
    virtual std::string getCommandSeparator() const = 0;
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
};
#endif

// ==================== Command Builder ====================
class CommandBuilder {
private:
    std::shared_ptr<PlatformStrategy> strategy;
    std::string fileName;
    std::string fileNameWithoutExt;
    std::string linkOptions;
    std::string currentDirName;
    std::string separator;

public:
    CommandBuilder(std::shared_ptr<PlatformStrategy> strat, 
                   const std::string& file, 
                   const std::string& options,
                   const std::string& currentDir)
        : strategy(strat)
        , fileName(file)
        , linkOptions(options)
        , currentDirName(currentDir) {
        
        std::filesystem::path filePath(fileName);
        fileNameWithoutExt = filePath.stem().string();
        separator = strategy->getCommandSeparator();
    }

    std::string build() {
        std::string runCmd;
        const char sep = strategy->pathSeparator;
        std::string ext = OS == "windows" ? ".exe" : "";
        
        if (currentDirName == "src") {
            runCmd = "g++ \"" + fileName + "\" -o \".." + sep + "bin" + sep + 
                     fileNameWithoutExt + ext + "\" " + linkOptions + " " + 
                     separator + " \".." + sep + "bin" + sep + 
                     fileNameWithoutExt + ext + "\"";
        } else {
            runCmd = "g++ \"" + fileName + "\" -o \"" + fileNameWithoutExt + 
                     ext + "\" " + linkOptions + " " + separator + " \"" + 
                     fileNameWithoutExt + ext + "\"";
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
        std::cout << "Enter the file name: ";
        std::getline(std::cin, fileName);
    }

    // Ask for link options
    std::string linkOptions;
    std::string fileNameWithoutExt = fileName.substr(0, fileName.find_last_of("."));
    std::string ext = fileName.substr(fileName.find_last_of(".") + 1);
    std::cout << "Enter link options for " << 
        Colored::text({Red, LightAqua}, fileNameWithoutExt, {LightGreen, LightGreen}) 
        << "." << Colored::text({LightAqua, Red}, ext) << ": ";
    std::getline(std::cin, linkOptions);

    // Create platform-specific strategy
    auto strategy = PlatformFactory::createStrategy();
    
    // Get current directory name
    std::string currentDirName = std::filesystem::current_path().filename().string();

    // Build command
    CommandBuilder builder(strategy, fileName, linkOptions, currentDirName);
    std::string runCmd = builder.build();
    
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