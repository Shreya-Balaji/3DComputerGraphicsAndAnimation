#pragma once
#include "bezier_curve.h"
#include "glm/ext/vector_float3.hpp"
#include <glm/glm.hpp>
#include <vector>

class CompositeBezierCurve {
    public:
        explicit CompositeBezierCurve(std::vector<BezierCurve> curves);

        ~CompositeBezierCurve() = default;

        CompositeBezierCurve(const CompositeBezierCurve&) = delete;
        CompositeBezierCurve& operator=(const CompositeBezierCurve&) = delete;

        glm::vec3 evaluate (float tValue) const;
        void draw(const glm::mat4 viewProjection) const;
        int curveCount() const;

    private:
        std::vector<BezierCurve> m_curves;
};
