#include "menu.h"
#include "config.h"
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <iterator>
#include <cmath>

Menu::Menu(Config& config, float cameraLoopLength)
    : m_config(config)
    , m_cameraLoopLength(cameraLoopLength)
{
}

void Menu::draw()
{
    if (ImGui::Begin("Camera View Points Controls")) {
        ImGui::Checkbox("Free Camera", &m_config.freeCameraEnabled);

        static constexpr const char* modes[] = {"Orbit View", "Top View", "Follow View", "Bezier View"};
        ImGui::Combo("Camera Mode", &m_config.cameraMode, modes, static_cast<int>(std::size(modes)));

        ImGui::Separator();
        if (m_config.cameraMode == 0 && !m_config.freeCameraEnabled) {
            ImGui::Text("Orbit Camera Settings");
            ImGui::SliderFloat("Yaw", &m_config.yaw, Config::MIN_YAW, Config::MAX_YAW, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Pitch", &m_config.pitch, Config::MIN_PITCH, Config::MAX_PITCH, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Distance", &m_config.distance, Config::MIN_DISTANCE, Config::MAX_DISTANCE, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        }
        if (m_config.cameraMode == 3 && !m_config.freeCameraEnabled) {
            ImGui::Text("Bezier Camera Settings");
            ImGui::SliderFloat("Loop distance", &m_config.cameraLoopDistance, 0.0f, m_cameraLoopLength, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Loop speed", &m_config.cameraLoopSpeed, -10.0f, 10.0f, "%.2f units/s", ImGuiSliderFlags_AlwaysClamp);
            ImGui::Checkbox("Draw Bezier curve", &m_config.drawBezierCurve);
        }
        ImGui::Separator();
        ImGui::Checkbox("Use PBR shading", &m_config.usePBR);
        if(m_config.usePBR == true){
            ImGui::SliderFloat("Metallic", &m_config.pbrMetallic, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Roughness", &m_config.pbrRoughness, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        }
        ImGui::Checkbox("Use Environment Reflections", &m_config.useEnvMap);
        ImGui::Separator();
        ImGui::Checkbox("Show minimap", &m_config.showMinimap);
        ImGui::SliderFloat("Open doors", &m_config.doorSlideFactor, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Wheel speed", &m_config.wheelSpeed, Config::MIN_WHEEL_SPEED, Config::MAX_WHEEL_SPEED, "%.0f deg/s", ImGuiSliderFlags_AlwaysClamp);
        if (m_config.freeCameraEnabled)
            ImGui::Text("WASD: move | Space/Ctrl: up/down | Left-drag: look");
        ImGui::Text("Cable Car Route");
        ImGui::SliderFloat("Cable car position", &m_config.cableCarT, 0.0f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();
        ImGui::Text("Snow Settings");
        ImGui::Checkbox("Snow enabled", &m_config.snowEnabled);
        ImGui::SliderFloat("Snow fall speed", &m_config.snowFallSpeed, 0.0f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Snow wind", &m_config.snowWind, -5.0f, 5.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Snow size", &m_config.snowSize, 1.0f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();
        ImGui::Text("Terrain Settings");
        ImGui::Checkbox("Terrain enabled", &m_config.terrainEnabled);
        ImGui::Checkbox("Two-scale sampling", &m_config.terrainTwoScaleSampling);
        ImGui::SliderFloat("Terrain speed", &m_config.terrainSpeed, -15.0f, 15.0f, "%.2f units/s", ImGuiSliderFlags_AlwaysClamp);

        ImGui::Separator();
        ImGui::Text("Sun and Moon Light Settings");
        ImGui::SliderFloat("Time of the Day", &m_config.timeOfDay, 0.0f, 1.0f, "", ImGuiSliderFlags_AlwaysClamp);
        const float hours = std::fmod(6.0f + 24.0f * m_config.timeOfDay, 24.0f);
        const int hour = static_cast<int>(hours);
        const int minute = static_cast<int>((hours - hour) * 60.0f);
        ImGui::SameLine();
        ImGui::Text("%02d:%02d", hour, minute);
        ImGui::ColorEdit3("Day sky", glm::value_ptr(m_config.dayColor));
        ImGui::ColorEdit3("Dusk", glm::value_ptr(m_config.duskColor));
        ImGui::ColorEdit3("Night sky", glm::value_ptr(m_config.nightColor));

        ImGui::Separator();
        ImGui::Checkbox("Cabin Light Enabled", &m_config.cabinLightEnabled);
        if(m_config.cabinLightEnabled){
            ImGui::ColorEdit3("Cabin Light Color", glm::value_ptr(m_config.cabinLightColor));
            ImGui::SliderFloat("Light Coverage Radius", &m_config.cabinLightRadius, 0.0f, 20.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Light Intensity", &m_config.cabinLightIntensity, 0.0f, 20.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
        }
        ImGui::Separator();
        ImGui::Checkbox("Cable Car Shadow Enabled", &m_config.shadowsEnabled);


    }
    ImGui::End();
}
