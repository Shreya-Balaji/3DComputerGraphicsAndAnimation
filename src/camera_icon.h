#pragma once

#include <framework/shader.h>
#include <glm/glm.hpp>

class CameraIcon {
public:
    CameraIcon();
    ~CameraIcon();
    CameraIcon(const CameraIcon&) = delete;
    CameraIcon& operator=(const CameraIcon&) = delete;

    void draw(const glm::mat4& viewProjection,
              const glm::mat4& cameraView) const;

private:
    Shader m_shader;
    GLint m_mvpLocation = -1;
    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLsizei m_vertexCount = 0;
};
