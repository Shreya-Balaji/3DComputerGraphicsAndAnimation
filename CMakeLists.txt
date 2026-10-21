cmake_minimum_required(VERSION 3.24)

project(CableCarAlps LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(framework)

find_package(assimp CONFIG REQUIRED)

add_executable(CableCarAlps
    src/main.cpp
    src/bezier_curve.cpp
    src/camera.cpp
    src/mesh.cpp
    src/model.cpp
)

target_link_libraries(CableCarAlps PRIVATE
    CGFramework
    assimp::assimp
)

target_compile_definitions(CableCarAlps PRIVATE
    PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)
