#include "menu.h"
#include "config.h"

#include <imgui/imgui.h>
#include <iterator>

Menu::Menu(Config& config, float maxCameraT)
    : m_config(config)
    , m_maxCameraT(maxCameraT)
{
}

void Menu::draw()
{
    if (ImGui::Begin("Camera View Points Controls")) {
        ImGui::Checkbox("Free Camera", &m_config.freeCameraEnabled);

        static constexpr const char* modes[] = {"Orbit View", "Top View", "Follow View", "Bezier View"};
        ImGui::Combo("Camera Mode", &m_config.cameraMode, modes, static_cast<int>(std::size(modes)));

        ImGui::Separator();
        ImGui::Text("Orbit Camera Settings");
        ImGui::SliderFloat("Yaw", &m_config.yaw, Config::MIN_YAW, Config::MAX_YAW, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Pitch", &m_config.pitch, Config::MIN_PITCH, Config::MAX_PITCH, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Distance", &m_config.distance, Config::MIN_DISTANCE, Config::MAX_DISTANCE, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Text("Bezier Camera Settings");
        ImGui::SliderFloat("t value", &m_config.tValue, 0.0f, m_maxCameraT, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Checkbox("Draw Bezier curve", &m_config.drawBezierCurve);
        ImGui::Checkbox("Show minimap", &m_config.showMinimap);
        ImGui::SliderFloat("Open doors", &m_config.doorSlideFactor, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Wheel speed", &m_config.wheelSpeed, Config::MIN_WHEEL_SPEED, Config::MAX_WHEEL_SPEED, "%.0f deg/s", ImGuiSliderFlags_AlwaysClamp);
        if (m_config.freeCameraEnabled)
            ImGui::Text("WASD: move | Space/Ctrl: up/down | Left-drag: look");
        ImGui::Text("Cable Car Route");
        ImGui::SliderFloat("Cable car position", &m_config.cableCarT, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
    }
    ImGui::End();
}
