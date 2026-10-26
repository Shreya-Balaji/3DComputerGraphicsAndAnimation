#include "minimap.h"
#include <glm/gtc/matrix_transform.hpp>

Minimap::~Minimap()
{
	if (m_depthBuffer != 0)
		glDeleteRenderbuffers(1, &m_depthBuffer);
	
	if (m_colorTexture != 0)
		glDeleteTextures(1, &m_colorTexture);

	if (m_framebuffer != 0)
		glDeleteFramebuffers(1, &m_framebuffer);

    if (m_overlayVBO != 0)
        glDeleteBuffers(1, &m_overlayVBO);

    if (m_overlayVAO != 0)
        glDeleteVertexArrays(1, &m_overlayVAO);
}
bool Minimap::initialize(int width, int height)
{
    if (width <= 0 || height <= 0)
        return false;

    m_width = width;
    m_height = height;

    ShaderBuilder builder;
    builder.addStage(GL_VERTEX_SHADER, "shaders/minimap.vert");
    builder.addStage(GL_FRAGMENT_SHADER, "shaders/minimap.frag");
    m_overlayShader = builder.build();

    const float vertices[] = {
         0.0f,  0.0f,     0.0f, 0.0f,
         1.0f,  0.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,

         0.0f,  0.0f,     0.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,
         0.0f,  1.0f,     0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_overlayVAO);
    glGenBuffers(1, &m_overlayVBO);

    glBindVertexArray(m_overlayVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_overlayVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float), nullptr
    );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        4 * sizeof(float),
        reinterpret_cast<void*>(2 * sizeof(float))
    );

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


       glGenFramebuffers(1, &m_framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);

    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB8,
        m_width,
        m_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        nullptr
    );

    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MIN_FILTER,
        GL_LINEAR
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_MAG_FILTER,
        GL_LINEAR
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_S,
        GL_CLAMP_TO_EDGE
    );
    glTexParameteri(
        GL_TEXTURE_2D,
        GL_TEXTURE_WRAP_T,
        GL_CLAMP_TO_EDGE
    );

    glFramebufferTexture2D(
        GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0,
        GL_TEXTURE_2D,
        m_colorTexture,
        0
    );

    glGenRenderbuffers(1, &m_depthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthBuffer);

    glRenderbufferStorage(
        GL_RENDERBUFFER,
        GL_DEPTH_COMPONENT24,
        m_width,
        m_height
    );

    glFramebufferRenderbuffer(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_RENDERBUFFER,
        m_depthBuffer
    );

    const bool complete =
        glCheckFramebufferStatus(GL_FRAMEBUFFER)
        == GL_FRAMEBUFFER_COMPLETE;

    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (!complete)
    {
      
        glDeleteRenderbuffers(1, &m_depthBuffer);
        glDeleteTextures(1, &m_colorTexture);
        glDeleteFramebuffers(1, &m_framebuffer);

        m_depthBuffer = 0;
        m_colorTexture = 0;
        m_framebuffer = 0;

        return false;
    }

    return true;
}

void Minimap::bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
    glViewport(0, 0, m_width, m_height);
}

void Minimap::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


glm::mat4 Minimap::viewMatrix() const
{
    return glm::lookAt(
        glm::vec3(-3.0f, 70.0f, -5.0f),
        glm::vec3(-3.0f, 0.0f, -5.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
    );
}


glm::mat4 Minimap::projectionMatrix() const
{
    return glm::ortho(
        -6.0f, 6.0f,
        -6.0f, 6.0f,
        0.1f, 150.0f
    );
}
void Minimap::drawOverlay(int windowWidth, int windowHeight)
{
    if (windowWidth <= 0 || windowHeight <= 0)
        return;

    const int margin = 20;
    const int overlaySize = 350;

    const int x = windowWidth - overlaySize - margin;
    const int y = windowHeight - overlaySize - margin;

    GLint previousViewport[4];
    glGetIntegerv(GL_VIEWPORT, previousViewport);

    const GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);

    glViewport(x, y, overlaySize, overlaySize);
    glDisable(GL_DEPTH_TEST);

    m_overlayShader.bind();
    

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);

    const GLint textureLocation =
        m_overlayShader.getUniformLocation("minimapTexture");

    if (textureLocation >= 0)
        glUniform1i(textureLocation, 0);

    glBindVertexArray(m_overlayVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);

    if (depthTestEnabled)
        glEnable(GL_DEPTH_TEST);

    glViewport(
        previousViewport[0],
        previousViewport[1],
        previousViewport[2],
        previousViewport[3]
    );
}