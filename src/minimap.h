
#pragma once

#include <framework/opengl_includes.h>
#include <framework/shader.h>
#include <glm/glm.hpp>
#include <framework/shader.h>

class Minimap
{
public:
    Minimap() = default;
    ~Minimap();

    Minimap(const Minimap&) = delete;
    Minimap& operator=(const Minimap&) = delete;

    bool initialize(int width = 512, int height = 512);

    void bind();
    void unbind();

    
    void drawOverlay(int windowWidth, int windowHeight);

    GLuint texture() const { return m_colorTexture; }

    glm::mat4 viewMatrix() const;
    glm::mat4 projectionMatrix() const;

private:
    GLuint m_framebuffer{ 0 };
    GLuint m_colorTexture{ 0 };
    GLuint m_depthBuffer{ 0 };

    
    GLuint m_overlayVAO{ 0 };
    GLuint m_overlayVBO{ 0 };
    Shader m_overlayShader;

    int m_width{ 512 };
    int m_height{ 512 };
};
