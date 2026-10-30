#pragma once

#include "composite_bezier_curve.h"
#include "config.h"
#include <glm/glm.hpp>

class Window;
class CableCar;

class AttachedCamera {
public:
    AttachedCamera(Window& window, const CableCar& cableCar, Config& config);
    AttachedCamera(const AttachedCamera&) = delete;
    AttachedCamera& operator=(const AttachedCamera&) = delete;

    glm::vec3 position() const;
    glm::mat4 viewMatrix() const;
    void updateInput();
    float maxTValue() const { return static_cast<float>(m_curve.curveCount()); }
    const CompositeBezierCurve& curve() const { return m_curve; }

private:
    void mouseButtonCallback(int button, int action);
    void mouseMoveCallback(const glm::vec2& position);
    void scrollCallback(const glm::vec2& offset);

    Window& m_window;
    Config& m_config;
    const CableCar& m_cableCar;
    glm::vec2 m_lastMouse{0.0f};
    bool m_firstMouse = true;
    bool m_dragging = false;
    CompositeBezierCurve m_curve;
};
