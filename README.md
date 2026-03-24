# CompileAndRun

A cross-platform C++ utility that compiles and runs C++ source files with user-specified link options. It uses platform-specific strategies to handle differences between Windows and Linux environments, providing colored output and seamless terminal integration.

## Features

- **Cross-Platform Support**: Works on both Windows and Linux using the Strategy design pattern
- **Dynamic Compilation**: Compiles and runs C++ files on-the-fly with custom link options
- **Colored Output**: Utilizes the `colorline` module for enhanced terminal output
- **Terminal Integration**: Automatically detects and uses the appropriate terminal (PowerShell/cmd on Windows, gnome-terminal/xterm on Linux)
- **CMake-Based Build**: Modern build system with presets and FetchContent for dependency management
- **C++23 Standard**: Leverages the latest C++ features and modules

## Prerequisites

- **CMake**: Version 3.28 or higher
- **C++ Compiler**: Supporting C++23 (GCC 13+, Clang 16+, or MSVC 19.35+)
- **Build System**: Ninja (recommended) or Make
- **Git**: For fetching dependencies

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/huseyinsc/compile_and_run.git
   cd compile_and_run
   ```

2. Build the project:
   - **Linux/macOS**:
     ```bash
     ./build.sh
     ```
   - **Windows** (PowerShell):
     ```powershell
     .\build.ps1
     ```

## Build Options

The build scripts support several options:

- `--clean` / `-Clean`: Clean the build directory before building
- `--debug`: Build in Debug mode (Release is default)
- `--static`: Create a static executable
- `--purge-cache`: Clear the global FetchContent cache (use with caution)
- `--install` / `-Install`: Automatically install the executable to your system PATH
- `--help` / `-Help`: Display help information and usage examples

### Automated Installation with `--install`

The easiest way to add `compile_and_run` to your PATH is to use the `--install` flag during the build process:

**Linux/macOS:**

```bash
./build.sh --install
# This builds and installs to /usr/local/bin (requires sudo)
```

**Windows (PowerShell):**

```powershell
.\build.ps1 -Install
# This builds and installs to %USERPROFILE%\bin
```

This will:
1. Build the project in Release mode
2. Automatically copy the executable to your PATH
3. Make `compile_and_run` available globally

After installation, you can use `compile_and_run` from any terminal or via CodeRunner.

### Manual PATH Installation

If you prefer to install manually after building:

Linux/macOS:

```bash
# Build first
./build.sh

# Copy to PATH directory
sudo cp build/compile_and_run /usr/local/bin/
sudo chmod +x /usr/local/bin/compile_and_run
```

Windows (PowerShell):

```powershell
# Build first
.\build.ps1

# Copy to bin directory
$binPath = "$env:USERPROFILE\bin"
if (-not (Test-Path $binPath)) { New-Item -ItemType Directory -Path $binPath -Force }
Copy-Item -Path .\build\compile_and_run.exe -Destination $binPath

# Make PATH persistent
[Environment]::SetEnvironmentVariable("Path", $env:Path + ";$binPath", "User")
```

After installation, restart VS Code to pick up the updated PATH and use CodeRunner.

### Build Examples

```bash
# Clean build in Debug mode
./build.sh --clean --debug

# Static release build with automatic installation
./build.sh --static --install

# Display help
./build.sh --help

# Windows equivalent
.\build.ps1 -Clean -Debug
.\build.ps1 -Static -Install
.\build.ps1 -Help
```

## Using CMake Presets

The project includes CMake presets for different build configurations. You can use them directly with CMake:

### Configure Presets

- `default`: Release build with Ninja generator, C++23 standard
- `debug`: Inherits from default, sets Debug build type
- `static`: Inherits from default, enables static linking

### Build Presets

- `default`: Builds using the default configure preset

To use presets:

```bash
# Configure with default preset
cmake --preset default

# Build with default preset
cmake --build --preset default

# Configure and build in one command
cmake --preset debug && cmake --build --preset debug
```

## Usage

Run the compiled executable:

```bash
./build/compile_and_run [filename.cpp]
```

If no filename is provided as an argument, the program will prompt for input.

### CodeRunner Integration (VS Code)

To use this project with the CodeRunner extension, add these to your Code Runner executor map in `settings.json`:

```json
"code-runner.executorMap": {
  "c": "cd $dir && compile_and_run \"$fileName\"",
  "cpp": "cd $dir && compile_and_run \"$fileName\""
}
```

This avoids manually writing long compile commands for each file, and delegates the compile-and-run logic to your program.

### Making `compile_and_run` globally accessible (PATH)

The recommended way is to use the `--install` flag during build (see [Automated Installation with `--install`](#automated-installation-with---install) above). This automatically handles all setup for you.

After installation with `--install`, verify it works:

```bash
# Test from any directory
compile_and_run --help

# Use in CodeRunner (see CodeRunner Integration section)
```

If you prefer manual installation, see the previous section for step-by-step instructions.

## Build Options

### Interactive Mode

1. Enter the C++ source file name (e.g., `hello.cpp`)
2. Enter any link options (e.g., `-lm` for math library, or leave empty)
3. The program will:
   - Detect the current directory context
   - Build the appropriate compilation command
   - Execute it in a new terminal window
   - Display colored output showing the parent process and command

### Example

```bash
$ ./build/compile_and_run
Enter the file name: example.cpp
Enter link options for example.cpp: -lstdc++fs
(gnome-terminal) Executing: gnome-terminal -- bash -c "g++ -std=c++23 example.cpp -lstdc++fs -o example && ./example; read -p 'Press Enter to exit...'"
```

## Project Structure

```
compile_and_run/
├── compile_and_run.cpp    # Main application source
├── CMakeLists.txt         # CMake build configuration
├── CMakePresets.json      # CMake presets for different build configurations
├── build.sh               # Linux/macOS build script
├── build.ps1              # Windows build script
├── build/                 # Build output directory (generated)
│   ├── compile_and_run    # Executable
│   └── ...                # Other build artifacts
└── README.md             # This file
```

## Dependencies

This project uses CMake's FetchContent to automatically download and build dependencies:

- **colorline**: Colored text output module
- **terminal_utils**: Terminal utility functions

Dependencies are fetched from: https://github.com/huseyinsc/cpp-modules

The global cache location:

- Windows: `%LOCALAPPDATA%\cpp-modules`
- Linux/macOS: `~/.cache/cpp-modules`

## Architecture

The application implements the Strategy pattern for platform-specific behavior:

- **PlatformStrategy**: Abstract interface for platform operations
- **WindowsStrategy**: Windows-specific implementation (PowerShell/cmd detection)
- **LinuxStrategy**: Linux-specific implementation (terminal detection)
- **CommandBuilder**: Constructs compilation commands based on platform and context
- **PlatformFactory**: Creates appropriate strategy instances

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test on both Windows and Linux
5. Submit a pull request

## License

This project is open source. Please check the repository for license information.

## Troubleshooting

- **Build fails**: Ensure you have CMake 3.28+ and a C++23 compatible compiler
- **Missing dependencies**: The build scripts will automatically download required modules
- **Terminal not detected**: The program falls back to default terminals (cmd on Windows, gnome-terminal on Linux)
- **Cache issues**: Use `--purge-cache` to clear and redownload dependencies
