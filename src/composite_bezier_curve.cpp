#include "composite_bezier_curve.h"

#include <glm/common.hpp>
#include <glm/geometric.hpp>
#include <algorithm>
#include <cmath>
#include <utility>
#include <stdexcept>
//  Construct composite Bezier curve from multiple individual Bezier segments.
CompositeBezierCurve::CompositeBezierCurve(std::vector<BezierCurve> curves)
    : m_curves(std::move(curves))
{
    if (m_curves.empty())
        throw std::invalid_argument("A composite curve needs at least one segment");
    buildArcLengthTable();
}
// Evaluate composite curve at a parameter value.
glm::vec3 CompositeBezierCurve::evaluate(float tValue) const {
    if (tValue == m_curves.size()) {
        return m_curves[m_curves.size()-1].evaluate(1.0f);
    }
     float integerPart;
     float fractionalPart = std::modf(tValue, &integerPart);
     int index = static_cast<int>(integerPart);

     return m_curves[index].evaluate(fractionalPart);
}
// Draw every Bezier segment
void CompositeBezierCurve::draw(const glm::mat4 viewProjection) const {
    for (const BezierCurve& curve : m_curves) {
        curve.draw(viewProjection);
    }
}

int CompositeBezierCurve::curveCount() const {
    return m_curves.size();
}

// table of arc lengths for reparametrization
void CompositeBezierCurve::buildArcLengthTable()
{
    constexpr int samplesPerCurve = 200;

    m_arcLengths.clear();
    m_arcLengths.push_back({0.0f, 0.0f});

    glm::vec3 previous = evaluate(0.0f);
    float distance = 0.0f;

    for (int curve = 0; curve < curveCount(); ++curve) {
        for (int i = 1; i <= samplesPerCurve; ++i) {
            float tValue = static_cast<float>(curve)
                + static_cast<float>(i) / samplesPerCurve;

            glm::vec3 current = evaluate(tValue);
            distance += glm::length(current - previous);

            m_arcLengths.push_back({tValue, distance});
            previous = current;
        }
    }
}
// Evaluate composite curve at a specified travelled distance.
glm::vec3 CompositeBezierCurve::evaluateAtDistance(float distance) const
{
    float totalLength = m_arcLengths.back().distance;
    distance = glm::clamp(distance, 0.0f, totalLength);

    // find the least sample that is greater than the distance
    auto upper = std::lower_bound(
        m_arcLengths.begin(),
        m_arcLengths.end(),
        distance,
        [](const ArcLengthSample& sample, float value) {
            return sample.distance < value;
        }
    );

    if (upper == m_arcLengths.begin())
        return evaluate(upper->tValue);

    if (upper == m_arcLengths.end())
        return evaluate(m_arcLengths.back().tValue);

    // the actual distance must be between upper and lower
    const auto& lower = *(upper - 1);

    float span = upper->distance - lower.distance;
    float fraction = span > 0.0f
        ? (distance - lower.distance) / span
        : 0.0f;

    // linearly interpolate the t value between the bounds
    float tValue = glm::mix(lower.tValue, upper->tValue, fraction);
    return evaluate(tValue);
}
