#pragma once
#include <glm/glm.hpp>

struct Config {
    static constexpr float MIN_YAW = -180.0f;
    static constexpr float MAX_YAW = 180.0f;
    static constexpr float MIN_PITCH = -85.0f;
    static constexpr float MAX_PITCH = 85.0f;
    static constexpr float MIN_DISTANCE = 2.0f;
    static constexpr float MAX_DISTANCE = 30.0f;
    static constexpr float MIN_WHEEL_SPEED = -20.0f;
    static constexpr float MAX_WHEEL_SPEED = 20.0f;

    bool freeCameraEnabled = false;
    int cameraMode = 0;
    float yaw = 25.0f;
    float pitch = 15.0f;
    float distance = 7.0f;
    float cameraLoopDistance = 0.0f;
    float cameraLoopSpeed = 2.0f; // World units per second; negative reverses direction.
    bool drawBezierCurve = false;
    bool showMinimap = true;
    float doorSlideFactor = 0.0f;
    bool doorWindowsOpen = false;
    float wheelSpeed = -20.0f; // Degrees per second.
    bool useEnvMap = true;
    bool usePBR = true;
    float pbrMetallic = 0.25f;
    float pbrRoughness = 0.25f;
    float timeOfDay = 0.25f; // 0.0 = sunrise, 0.25 = midday, 0.5 = sunset, 0.75 = midnight etc.

    glm::vec3 nightColor{0.0f, 0.0f, 0.03f};
    glm::vec3 duskColor{0.9f, 0.5f, 0.3f};
    glm::vec3 dayColor{0.25f, 0.45f, 0.65f};

    bool cabinLightEnabled = true;
    glm::vec3 cabinLightColor{1.0f, 0.75f, 0.4f};
    float cabinLightRadius = 10.0f;
    float cabinLightIntensity = 10.0f;

    bool snowEnabled = true;
    float snowFallSpeed = 1.5f;
    float snowWind = 0.2f;
    float snowSize = 4.0f; // Pixel size at a distance of 12 world units.

    bool terrainEnabled = true;
    bool terrainTwoScaleSampling = true;
    float terrainSpeed = 3.0f; // World units per second; negative reverses travel.
    bool shadowsEnabled = true;
};
