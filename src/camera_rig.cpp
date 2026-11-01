#include "camera_rig.h"

CameraRig::CameraRig(Window& window, const CableCar& cableCar, Config& config)
    : m_config(config)
    , m_attachedCamera(window, cableCar, config)
    , m_freeCamera(&window, config, glm::vec3(1.5f, 1.0f, 1.0f),
                   -glm::vec3(1.5f, 1.0f, 1.0f))
{
}

void CameraRig::updateInput(bool guiCapturesInput)
{
    m_attachedCamera.updateInput();
    m_freeCamera.updateInput(guiCapturesInput);
}

glm::vec3 CameraRig::position() const
{
    return m_config.freeCameraEnabled ? m_freeCamera.cameraPos() : m_attachedCamera.position();
}

glm::mat4 CameraRig::viewMatrix() const
{
    return m_config.freeCameraEnabled ? m_freeCamera.viewMatrix() : m_attachedCamera.viewMatrix();
}
