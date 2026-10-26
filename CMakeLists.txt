cmake_minimum_required(VERSION 3.24)

project(CableCarAlps LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(framework)

find_package(assimp CONFIG REQUIRED)

add_executable(CableCarAlps
    src/main.cpp
    src/cable_car.cpp
    src/environment.cpp
    src/bezier_curve.cpp
    src/composite_bezier_curve.cpp
    src/mesh.cpp
    src/attached_camera.cpp
    src/camera_icon.cpp
    src/camera_rig.cpp
    src/free_camera.cpp
    src/assimp_utils.cpp
    src/model.cpp
    src/minimap.cpp
)

target_link_libraries(CableCarAlps PRIVATE
    CGFramework
    assimp::assimp
)

target_compile_definitions(CableCarAlps PRIVATE
    PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)
