#include "free_camera.h"
#include <framework/window.h>
// Suppress warnings in third-party code.
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
DISABLE_WARNINGS_POP()

FreeCamera::FreeCamera(Window* pWindow, const Config& config, const glm::vec3& pos, const glm::vec3& forward)
    : m_position(pos)
    , m_forward(glm::normalize(forward))
    , m_pWindow(pWindow)
    , m_config(config)
{
}

glm::vec3 FreeCamera::cameraPos() const
{
    return m_position;
}

glm::mat4 FreeCamera::viewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

void FreeCamera::rotateX(float angle)
{
    const glm::vec3 horAxis = glm::cross(s_yAxis, m_forward);

    m_forward = glm::normalize(glm::angleAxis(angle, horAxis) * m_forward);
    m_up = glm::normalize(glm::cross(m_forward, horAxis));
}

void FreeCamera::rotateY(float angle)
{
    const glm::vec3 horAxis = glm::cross(s_yAxis, m_forward);

    m_forward = glm::normalize(glm::angleAxis(angle, s_yAxis) * m_forward);
    m_up = glm::normalize(glm::cross(m_forward, horAxis));
}

void FreeCamera::updateInput(bool guiCapturesInput)
{
    constexpr float moveSpeed = 0.05f;
    constexpr float lookSpeed = 0.0035f;

    const bool enabled = m_config.freeCameraEnabled && !guiCapturesInput;
    if (enabled && !m_userInteraction)
        m_prevCursorPos = m_pWindow->getCursorPos();
    m_userInteraction = enabled;

    if (m_userInteraction) {
        glm::vec3 localMoveDelta{ 0 };
        const glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up));
        if (m_pWindow->isKeyPressed(GLFW_KEY_A))
            m_position -= moveSpeed * right;
        if (m_pWindow->isKeyPressed(GLFW_KEY_D))
            m_position += moveSpeed * right;
        if (m_pWindow->isKeyPressed(GLFW_KEY_W))
            m_position += moveSpeed * m_forward;
        if (m_pWindow->isKeyPressed(GLFW_KEY_S))
            m_position -= moveSpeed * m_forward;
        if (m_pWindow->isKeyPressed(GLFW_KEY_SPACE))
            m_position += moveSpeed * m_up;
        if (m_pWindow->isKeyPressed(GLFW_KEY_LEFT_CONTROL))
            m_position -= moveSpeed * m_up;

        const glm::dvec2 cursorPos = m_pWindow->getCursorPos();
        const glm::vec2 delta = lookSpeed * glm::vec2(cursorPos - m_prevCursorPos);
        m_prevCursorPos = cursorPos;

        if (m_pWindow->isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            if (delta.x != 0.0f)
                rotateY(delta.x);
            if (delta.y != 0.0f)
                rotateX(delta.y);
        }
    }
    else {
        m_prevCursorPos = m_pWindow->getCursorPos();
    }
}
