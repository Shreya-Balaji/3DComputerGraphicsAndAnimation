#include "camera_icon.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iterator>
#include <string>

CameraIcon::CameraIcon()
{
    ShaderBuilder builder;
    const std::string root = PROJECT_ROOT;
    builder.addStage(GL_VERTEX_SHADER, root + "/shaders/curve.vert");
    builder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/curve.frag");
    m_shader = builder.build();
    m_mvpLocation = m_shader.getUniformLocation("mvp");

    const glm::vec3 lines[] = {
        // rectangle
        {-1, -0.7f, -2}, { 1, -0.7f, -2},
        { 1, -0.7f, -2}, { 1,  0.7f, -2},
        { 1,  0.7f, -2}, {-1,  0.7f, -2},
        {-1,  0.7f, -2}, {-1, -0.7f, -2},
        // rectangle to source lines
        {0, 0, 0}, {-1, -0.7f, -2},
        {0, 0, 0}, { 1, -0.7f, -2},
        {0, 0, 0}, { 1,  0.7f, -2},
        {0, 0, 0}, {-1,  0.7f, -2},
        // triangle to show up direciton
        {-0.4f, 0.7f, -2}, {0, 1.2f, -2},
        {0, 1.2f, -2}, {0.4f, 0.7f, -2}
    };
    m_vertexCount = static_cast<GLsizei>(std::size(lines));
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

CameraIcon::~CameraIcon()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void CameraIcon::draw(const glm::mat4& viewProjection,
                      const glm::mat4& cameraView) const
{
    const glm::mat4 model = glm::scale(glm::inverse(cameraView), glm::vec3(0.2f));
    const glm::mat4 mvp = viewProjection * model;
    m_shader.bind();
    glUniformMatrix4fv(m_mvpLocation, 1, GL_FALSE, glm::value_ptr(mvp));
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINES, 0, m_vertexCount);
    glBindVertexArray(0);
}
