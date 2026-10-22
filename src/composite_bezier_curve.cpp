#include "composite_bezier_curve.h"
#include "bezier_curve.h"
#include <algorithm>
#include <cmath>
#include <utility>
#include <stdexcept>

CompositeBezierCurve::CompositeBezierCurve(std::vector<BezierCurve> curves)
    : m_curves(std::move(curves))
{
    if (m_curves.empty())
        throw std::invalid_argument("A composite curve needs at least one segment");
    buildArcLengthTable();
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

void CompositeBezierCurve::buildArcLengthTable()
{
    constexpr int samplesPerSegment = 200;

    m_arcLengths.clear();
    m_arcLengths.push_back({0.0f, 0.0f});

    glm::vec3 previous = evaluate(0.0f);
    float distance = 0.0f;

    for (int segment = 0; segment < curveCount(); ++segment) {
        for (int i = 1; i <= samplesPerSegment; ++i) {
            float tValue = static_cast<float>(segment)
                + static_cast<float>(i) / samplesPerSegment;

            glm::vec3 current = evaluate(tValue);
            distance += glm::length(current - previous);

            m_arcLengths.push_back({tValue, distance});
            previous = current;
        }
    }
}

glm::vec3 CompositeBezierCurve::evaluateAtDistance(float distance) const
{
    float totalLength = m_arcLengths.back().distance;
    distance = glm::clamp(distance, 0.0f, totalLength);

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

    const auto& lower = *(upper - 1);

    float span = upper->distance - lower.distance;
    float fraction = span > 0.0f
        ? (distance - lower.distance) / span
        : 0.0f;

    float tValue = glm::mix(lower.tValue, upper->tValue, fraction);
    return evaluate(tValue);
}
