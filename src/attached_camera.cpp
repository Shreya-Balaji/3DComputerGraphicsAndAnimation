#include "attached_camera.h"

#include <framework/window.h>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <iostream>


AttachedCamera::AttachedCamera(Window& window, const glm::vec3& target)
    : m_window(window), m_target(target)
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

void AttachedCamera::setTarget(const glm::vec3& target)
{
    m_target = target;
}

glm::vec3 AttachedCamera::position() const
{
    if (m_mode == 1)
        return m_target + glm::vec3(0.0f, 12.0f, 0.01f);
    if (m_mode == 2)
        return m_target + glm::vec3(0.0f, 2.5f, 8.0f);

    float yaw = glm::radians(m_yaw);
    float pitch = glm::radians(m_pitch);
    return m_target + glm::vec3(
        m_distance * std::cos(pitch) * std::sin(yaw),
        m_distance * std::sin(pitch),
        m_distance * std::cos(pitch) * std::cos(yaw)
    );
}

glm::mat4 AttachedCamera::viewMatrix() const
{
    return glm::lookAt(position(), m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void AttachedCamera::setUserInteraction(bool enabled)
{
    m_userInteraction = enabled;
    if (!enabled) {
        m_dragging = false;
        m_firstMouse = true;
    }
}

void AttachedCamera::updateInput()
{
    if (!m_userInteraction || !m_window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) ||
        ImGui::GetIO().WantCaptureMouse) {
        m_dragging = false;
        m_firstMouse = true;
    }
}

void AttachedCamera::setMode(int mode)
{
    if (mode < 0 || mode > MODE_COUNT - 1) {
        std::cerr << "Incorrect mode index";
    }
    m_mode = mode;
}

void AttachedCamera::setYaw(float yaw)
{
    m_yaw = yaw;
}

void AttachedCamera::setPitch(float pitch)
{
    m_pitch = glm::clamp(pitch, MIN_PITCH, MAX_PITCH);
}

void AttachedCamera::setDistance(float distance)
{
    m_distance = glm::clamp(distance, MIN_DISTANCE, MAX_DISTANCE);
}

void AttachedCamera::mouseButtonCallback(int button, int action)
{
    if (!m_userInteraction)
        return;
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        m_dragging = (action == GLFW_PRESS);
        m_firstMouse = true;
    }
}

void AttachedCamera::mouseMoveCallback(const glm::vec2& position)
{
    if (!m_userInteraction || !m_dragging)
        return;
    if (m_firstMouse) {
        m_lastMouse = position;
        m_firstMouse = false;
        return;
    }
    glm::vec2 delta = position - m_lastMouse;
    m_yaw += delta.x * 0.3f;
    m_pitch = glm::clamp(m_pitch + delta.y * 0.3f, MIN_PITCH, MAX_PITCH);
    m_lastMouse = position;
}

void AttachedCamera::scrollCallback(const glm::vec2& offset)
{
    if (!m_userInteraction)
        return;
    m_distance = glm::clamp(m_distance - offset.y * 0.5f, MIN_DISTANCE, MAX_DISTANCE);
}
