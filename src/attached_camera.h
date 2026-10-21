#pragma once

#include <glm/glm.hpp>
#include <iterator>

class Window;

// Keep this camera alive while the window processes its registered callbacks.
class AttachedCamera {
public:
    static constexpr const char* MODES[] = {"Orbit View", "Top View", "Follow View"};
    static constexpr int MODE_COUNT = static_cast<int>(std::size(MODES));

    static constexpr float MIN_YAW = -180.0f;
    static constexpr float MAX_YAW = 180.0f;
    static constexpr float MIN_PITCH = -85.0f;
    static constexpr float MAX_PITCH = 85.0f;
    static constexpr float MIN_DISTANCE = 2.0f;
    static constexpr float MAX_DISTANCE = 30.0f;

    AttachedCamera(Window& window, const glm::vec3& target);
    AttachedCamera(const AttachedCamera&) = delete;
    AttachedCamera& operator=(const AttachedCamera&) = delete;

    void setTarget(const glm::vec3& target);
    glm::vec3 position() const;
    glm::mat4 viewMatrix() const;
    void updateInput();
    int mode() const { return m_mode; }
    float yaw() const { return m_yaw; }
    float pitch() const { return m_pitch; }
    float distance() const { return m_distance; }
    void setMode(int mode);
    void setYaw(float yaw);
    void setPitch(float pitch);
    void setDistance(float distance);

private:
    void mouseButtonCallback(int button, int action);
    void mouseMoveCallback(const glm::vec2& position);
    void scrollCallback(const glm::vec2& offset);

    Window& m_window;
    glm::vec3 m_target;
    float m_yaw = 25.0f;
    float m_pitch = 15.0f;
    float m_distance = 7.0f;
    glm::vec2 m_lastMouse{0.0f};
    bool m_firstMouse = true;
    bool m_dragging = false;
    int m_mode = 0;
};
