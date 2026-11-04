# Cable Car Through the Alps

This is a A C++/OpenGL 4.1 project and the application features an endlessly scrolling cable car ride through a snowy mountain landscape, with a live control panel (ImGui).

## Installation and Setup

### 1. Prerequisites

- [Assimp](https://github.com/assimp/assimp), which is the only external dependency (GLFW, glad, glm, ImGui and the other libraries are bundled in `framework/third_party`)

On macOS, install Assimp with Homebrew:

```bash
brew install assimp
```

### 2. Configure the Project

From the project root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

### 3. Build the Project

```bash
cmake --build build --parallel
```

## Running the Project

```bash
./build/CableCarAlps
```

This starts the Cable Car application. Run it from the project root, so the `shaders/` and `assets/` folders are found.

## Common Issues

- **CMake cannot find Assimp:** add `-DCMAKE_PREFIX_PATH="$(brew --prefix)"` to the configure command.
- **Shaders or models are not found:** run the program from the project root.