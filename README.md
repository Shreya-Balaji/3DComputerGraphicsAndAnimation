# Cable Car Simulation

## Installation and Setup

The following instructions configure and build the project on macOS.

### 1. Install Prerequisites

Install Homebrew dependencies:

```bash
brew install glfw glm assimp ninja
```

Install vcpkg if it is not already installed:

```bash
git clone https://github.com/microsoft/vcpkg.git "$HOME/vcpkg"
"$HOME/vcpkg/bootstrap-vcpkg.sh"
```

If vcpkg is already installed, skip the cloning and bootstrap commands.

### 2. Install Project Dependencies

From the project root, run:

```bash
"$HOME/vcpkg/vcpkg" install
```

This installs the dependencies specified in the project's vcpkg configuration.

### 3. Set Up Dear ImGui

Clone Dear ImGui into the `external/imgui` directory:

```bash
mkdir -p external
git clone https://github.com/ocornut/imgui.git external/imgui
```

Skip this step if Dear ImGui has already been cloned into that directory.

### 4. Configure the Project

Configure the project using CMake and vcpkg. The build files will be generated in the `build` directory.

```bash
cmake -S . -B build \
  -DCMAKE_TOOLCHAIN_FILE="$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" \
  -DVCPKG_INSTALLED_DIR="$PWD/vcpkg_installed" \
  -Dglad_DIR="$PWD/vcpkg_installed/arm64-osx/share/glad" \
  -DCMAKE_BUILD_TYPE=Debug
```

### 5. Build the Project

Compile the source code and generate the executable:

```bash
cmake --build build
```

## Running the Project

After successfully building the project, run:

```bash
./build/CableCarAlps
```

This launches the Cable Car Simulation application.
