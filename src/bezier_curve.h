#pragma once

#include <glad/glad.h>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <vector>

class BezierCurve {
    public:
        BezierCurve(
            const glm::vec3& p0,
            const glm::vec3& p1,
            const glm::vec3& p2,
            const glm::vec3& p3,
            int segments = 100
        );

        ~BezierCurve();

        BezierCurve(const BezierCurve&) = delete;
        BezierCurve& operator=(const BezierCurve&) = delete;

        glm::vec3 evaluate(float t) const;
        void draw() const;
        std::vector<BezierCurve> fromControlPoints(const std::vector<glm::vec3>& controlPoints) const;

    private:
        glm::vec3 m_p0, m_p1, m_p2, m_p3;

        GLuint m_vao = 0;
        GLuint m_vbo = 0;
        GLuint m_pointCount = 0;
};
