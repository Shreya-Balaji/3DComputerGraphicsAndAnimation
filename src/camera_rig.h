#pragma once

#include "attached_camera.h"
#include "free_camera.h"

class CameraRig {
public:
    CameraRig(Window& window, const CableCar& cableCar, Config& config);

    void updateInput(bool guiCapturesInput);

    glm::vec3 position() const;
    glm::mat4 viewMatrix() const;
    const AttachedCamera& attachedCamera() const { return m_attachedCamera; }

private:
    const Config& m_config;
    AttachedCamera m_attachedCamera;
    FreeCamera m_freeCamera;
};
