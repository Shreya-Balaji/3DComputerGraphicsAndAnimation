#include "camera_rig.h"

CameraRig::CameraRig(Window& window, const glm::vec3& target)
    : m_attachedCamera(window, target)
    , m_freeCamera(&window, glm::vec3(1.5f, 1.0f, 1.0f),
                   -glm::vec3(1.5f, 1.0f, 1.0f))
{
    m_freeCamera.setUserInteraction(false);
}

void CameraRig::setFreeCameraEnabled(bool enabled)
{
    m_freeCameraEnabled = enabled;
    m_attachedCamera.setUserInteraction(!enabled);
    m_freeCamera.setUserInteraction(false);
}

void CameraRig::updateInput(bool guiCapturesInput)
{
    m_attachedCamera.updateInput();
    m_freeCamera.setUserInteraction(m_freeCameraEnabled && !guiCapturesInput);
    m_freeCamera.updateInput();
}

glm::vec3 CameraRig::position() const
{
    return m_freeCameraEnabled ? m_freeCamera.cameraPos() : m_attachedCamera.position();
}

glm::mat4 CameraRig::viewMatrix() const
{
    return m_freeCameraEnabled ? m_freeCamera.viewMatrix() : m_attachedCamera.viewMatrix();
}
