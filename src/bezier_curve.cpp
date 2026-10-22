#include <utility>
#include "bezier_curve.h"
#include "framework/shader.h"

#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <string>
#include <vector>
#include <iostream>

glm::vec3 BezierCurve::evaluate(float tValue) const
{
    float u = 1.0f - tValue;

    return u * u * u * m_p0
         + 3.0f * u * u * tValue * m_p1
         + 3.0f * u * tValue * tValue * m_p2
         + tValue * tValue * tValue * m_p3;
}

BezierCurve::BezierCurve(
    const glm::vec3& p0,
    const glm::vec3& p1,
    const glm::vec3& p2,
    const glm::vec3& p3,
    int segments)
    : m_p0(p0), m_p1(p1), m_p2(p2), m_p3(p3)
{
    if (segments < 1)
        throw std::invalid_argument("segments must be positive");

    std::vector<glm::vec3> points;

    for (int i = 0; i <= segments; ++i)
    {
        float tValue = static_cast<float>(i) / segments;
        points.push_back(evaluate(tValue));
    }

    m_pointCount = static_cast<GLsizei>(points.size());

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        points.size() * sizeof(glm::vec3),
        points.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), nullptr
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);

    try {
        std::string root = PROJECT_ROOT;
        ShaderBuilder curveBuilder;
        curveBuilder.addStage(GL_VERTEX_SHADER, static_cast<std::string>(PROJECT_ROOT) + "/shaders/curve.vert");
        curveBuilder.addStage(GL_FRAGMENT_SHADER, static_cast<std::string>(PROJECT_ROOT) + "/shaders/curve.frag");
        curveShader = curveBuilder.build();
    } catch (ShaderLoadingException e) {
        std::cerr << e.what() << std::endl;
    }
}

BezierCurve::BezierCurve(BezierCurve&& other) noexcept
    : m_p0(other.m_p0), m_p1(other.m_p1), m_p2(other.m_p2), m_p3(other.m_p3)
    , m_vao(std::exchange(other.m_vao, 0))
    , m_vbo(std::exchange(other.m_vbo, 0))
    , m_pointCount(std::exchange(other.m_pointCount, 0))
    , curveShader(std::move(other.curveShader))
{
}

BezierCurve::~BezierCurve()
{
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_vao);
}

void BezierCurve::draw(const glm::mat4 viewProjection) const
{
    curveShader.bind();
    glUniformMatrix4fv(
        curveShader.getUniformLocation("mvp"),
        1, GL_FALSE, glm::value_ptr(viewProjection)
    );
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINE_STRIP, 0, m_pointCount);
    glBindVertexArray(0);
}
