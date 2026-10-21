#pragma once

#include "attached_camera.h"
#include "free_camera.h"

class CameraRig {
public:
    CameraRig(Window& window, const glm::vec3& target);

    bool freeCameraEnabled() const { return m_freeCameraEnabled; }
    void setFreeCameraEnabled(bool enabled);
    void updateInput(bool guiCapturesInput);

    glm::vec3 position() const;
    glm::mat4 viewMatrix() const;
    AttachedCamera& attachedCamera() { return m_attachedCamera; }
    const AttachedCamera& attachedCamera() const { return m_attachedCamera; }

private:
    AttachedCamera m_attachedCamera;
    FreeCamera m_freeCamera;
    bool m_freeCameraEnabled = false;
};
