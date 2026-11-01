#pragma once

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
    float tValue = 0.0f;
    bool drawBezierCurve = false;
    bool showMinimap = true;
    float doorSlideFactor = 0.0f;
    float wheelSpeed = 10.0f; // Degrees per second.
    float cableCarT = 0.35f;
};
