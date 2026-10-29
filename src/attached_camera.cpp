#include "attached_camera.h"
#include "glm/common.hpp"

#include <framework/window.h>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>


AttachedCamera::AttachedCamera(Window& window, const glm::vec3& target, Config& config)
    : m_window(window)
    , m_config(config)
    , m_target(target)
    , m_curve([] {
        std::vector<BezierCurve> curves;
        curves.emplace_back(
            glm::vec3(-3.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 0.0f, 3.0f),
            glm::vec3( 1.0f, 0.0f, 3.0f),
            glm::vec3( 3.0f, 0.0f, 0.0f)
        );
        curves.emplace_back(
            glm::vec3( 3.0f, 0.0f,  0.0f),
            glm::vec3( 2.0f, 3.0f, -3.0f),
            glm::vec3(-3.0f, 4.0f, -5.0f),
            glm::vec3(-3.0f, 7.0f, -8.0f)
        );
        return curves;
    }())

{
    window.registerMouseButtonCallback([this](int button, int action, int) {
        mouseButtonCallback(button, action);
    });
    window.registerMouseMoveCallback([this](const glm::vec2& position) {
        mouseMoveCallback(position);
    });
    window.registerScrollCallback([this](const glm::vec2& offset) {
        scrollCallback(offset);
    });
}

glm::vec3 AttachedCamera::position() const
{
    if (m_config.cameraMode == 1)
        return m_target + glm::vec3(0.0f, 12.0f, 0.01f);
    if (m_config.cameraMode == 2)
        return m_target + glm::vec3(0.0f, 2.5f, 8.0f);
    if (m_config.cameraMode == 3)
        return m_curve.evaluate(m_config.tValue);

    float yaw = glm::radians(m_config.yaw);
    float pitch = glm::radians(m_config.pitch);
    return m_target + glm::vec3(
        m_config.distance * std::cos(pitch) * std::sin(yaw),
        m_config.distance * std::sin(pitch),
        m_config.distance * std::cos(pitch) * std::cos(yaw)
    );
}

glm::mat4 AttachedCamera::viewMatrix() const
{
    return glm::lookAt(position(), m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void AttachedCamera::updateInput()
{
    if (m_config.freeCameraEnabled || !m_window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ||
        ImGui::GetIO().WantCaptureMouse) {
        m_dragging = false;
        m_firstMouse = true;
    }
}

void AttachedCamera::mouseButtonCallback(int button, int action)
{
    if (m_config.freeCameraEnabled)
        return;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_dragging = (action == GLFW_PRESS);
        m_firstMouse = true;
    }
}

void AttachedCamera::mouseMoveCallback(const glm::vec2& position)
{
    if (m_config.freeCameraEnabled || !m_dragging)
        return;
    if (m_firstMouse) {
        m_lastMouse = position;
        m_firstMouse = false;
        return;
    }
    glm::vec2 delta = position - m_lastMouse;
    m_config.yaw += delta.x * 0.3f;
    m_config.pitch = glm::clamp(m_config.pitch + delta.y * 0.3f, Config::MIN_PITCH, Config::MAX_PITCH);
    m_lastMouse = position;
}

void AttachedCamera::scrollCallback(const glm::vec2& offset)
{
    if (m_config.freeCameraEnabled)
        return;
    m_config.distance = glm::clamp(m_config.distance - offset.y * 0.5f, Config::MIN_DISTANCE, Config::MAX_DISTANCE);
}
