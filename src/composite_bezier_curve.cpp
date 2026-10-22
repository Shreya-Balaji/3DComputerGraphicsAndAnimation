#include "composite_bezier_curve.h"
#include "bezier_curve.h"
#include <cmath>
#include <utility>
#include <stdexcept>

CompositeBezierCurve::CompositeBezierCurve(std::vector<BezierCurve> curves)
    : m_curves(std::move(curves))
{
    if (m_curves.empty())
        throw std::invalid_argument("A composite curve needs at least one segment");
}

glm::vec3 CompositeBezierCurve::evaluate(float tValue) const {
    if (tValue == m_curves.size()) {
        return m_curves[m_curves.size()-1].evaluate(1.0f);
    }
     float integerPart;
     float fractionalPart = std::modf(tValue, &integerPart);
     int index = static_cast<int>(integerPart);

     return m_curves[index].evaluate(fractionalPart);
}

void CompositeBezierCurve::draw(const glm::mat4 viewProjection) const {
    for (const BezierCurve& curve : m_curves) {
        curve.draw(viewProjection);
    }
}

int CompositeBezierCurve::curveCount() const {
    return m_curves.size();
}
