cmake_minimum_required(VERSION 3.24)

project(CableCarAlps LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

add_executable(CableCarAlps
    src/main.cpp
)

target_link_libraries(CableCarAlps PRIVATE
    glfw
    glad::glad
    glm::glm
    assimp::assimp
)

target_compile_definitions(CableCarAlps PRIVATE
    PROJECT_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)