#include "cable_car.h"
#include "environment.h"
#include "camera_rig.h"
#include "camera_icon.h"
#include "minimap.h"

#include <framework/window.h>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <exception>
#include <iostream>
#include <stdexcept>

glm::vec3 evaluateCableCarRoute(float t)
{
    // Route along the outside edge of the mountain.
    const glm::vec3 p0(-5.4f, 0.0f, -2.0f);
    const glm::vec3 p1(-5.7f, 0.7f, -3.8f);
    const glm::vec3 p2(-5.7f, 1.0f, -6.0f);
    const glm::vec3 p3(-5.4f, 0.4f, -8.0f);

    t = glm::clamp(t, 0.0f, 1.0f);
    const float u = 1.0f - t;

    return u * u * u * p0
        + 3.0f * u * u * t * p1
        + 3.0f * u * t * t * p2
        + t * t * t * p3;
}
int main()
{
    Window window(
        "Cable Car Through the Alps",
        glm::ivec2(1280, 720),
        OpenGLVersion::GL41
    );

    CameraRig cameraRig(window, glm::vec3(0.0f));
    AttachedCamera& attachedCamera = cameraRig.attachedCamera();

    bool drawBezierCurve = false;
    float doorSlideFactor = 0.0f;
    float cableCarT = 0.35f;
    bool showMinimap = true;
  

    glEnable(GL_DEPTH_TEST);

    try
    {
        CableCar cableCar;
      
        Environment environment;
        if (!environment.initialize())
        {
            throw std::runtime_error(
                "Could not initialize mountain environment."
            );
        }

        CameraIcon cameraIcon;

        Minimap minimap;
        if (!minimap.initialize(512, 512))
        {
            throw std::runtime_error(
                "Could not initialize minimap framebuffer."
            );
        }

        while (!window.shouldClose())
        {
            window.updateInput();
            if (window.isKeyPressed(GLFW_KEY_ESCAPE))
                window.close();

            auto size = window.getFrameBufferSize();
            glViewport(0, 0, size.x, size.y);

            glClearColor(0.25f, 0.45f, 0.65f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ImGui::Begin("Camera View Points Controls");

            bool freeCameraEnabled = cameraRig.freeCameraEnabled();
            if (ImGui::Checkbox("Free Camera", &freeCameraEnabled))
                cameraRig.setFreeCameraEnabled(freeCameraEnabled);

            int mode = attachedCamera.mode();
            if (ImGui::Combo("Camera Mode", &mode, AttachedCamera::MODES, AttachedCamera::MODE_COUNT))
                attachedCamera.setMode(mode);

            ImGui::Separator();
            ImGui::Text("Orbit Camera Settings");
            float yaw = attachedCamera.yaw();
            float pitch = attachedCamera.pitch();
            float distance = attachedCamera.distance();
            float tValue = attachedCamera.tValue();
            if (ImGui::SliderFloat("Yaw", &yaw, AttachedCamera::MIN_YAW, AttachedCamera::MAX_YAW))
                attachedCamera.setYaw(yaw);
            if (ImGui::SliderFloat("Pitch", &pitch, AttachedCamera::MIN_PITCH, AttachedCamera::MAX_PITCH))
                attachedCamera.setPitch(pitch);
            if (ImGui::SliderFloat("Distance", &distance, AttachedCamera::MIN_DISTANCE, AttachedCamera::MAX_DISTANCE))
                attachedCamera.setDistance(distance);
            ImGui::Text("Bezier Camera Settings");
            if (ImGui::SliderFloat("t value", &tValue, 0.0f, attachedCamera.maxTValue()))
                attachedCamera.setTValue(tValue);
            ImGui::Checkbox("Draw Bezier curve", &drawBezierCurve);
            ImGui::Checkbox("Show minimap", &showMinimap);
            if (ImGui::SliderFloat("Open doors", &doorSlideFactor, 0.0f, 1.0f))
                cableCar.setDoorSlideFactor(doorSlideFactor);
            if (freeCameraEnabled)
                ImGui::Text("WASD: move | Space/Ctrl: up/down | Left-drag: look");
            ImGui::Text("Cable Car Route");

            ImGui::SliderFloat(
                "Cable car position",
                &cableCarT,
                0.0f,
                1.0f
            );
            ImGui::End();
            
            const ImGuiIO& io = ImGui::GetIO();
            cameraRig.updateInput(io.WantCaptureMouse || io.WantCaptureKeyboard);

            const glm::mat4 view = cameraRig.viewMatrix();
            const glm::vec3 cameraPosition = cameraRig.position();

            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                1280.0f / 720.0f,
                0.1f,
                1000.0f
            );
            const glm::mat4 viewProjection = projection * view;

            const glm::vec3 carPosition =
                evaluateCableCarRoute(1.0f - cableCarT);

            cableCar.setWorldTransform(
                carPosition,
                0.55f,
                glm::vec3(0.0f)
            );
            cableCar.draw(viewProjection, cameraPosition);

            environment.draw(view, projection);

            if (showMinimap)
            {
                minimap.bind();

                glClearColor(0.25f, 0.45f, 0.65f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                const glm::mat4 minimapView = minimap.viewMatrix();
                const glm::mat4 minimapProjection = minimap.projectionMatrix();
                const glm::mat4 minimapViewProjection =
                    minimapProjection * minimapView;

                const glm::vec3 minimapCameraPosition(-3.0f, 70.0f, -5.0f);

                environment.draw(minimapView, minimapProjection);
                cableCar.draw(minimapViewProjection, minimapCameraPosition);

                minimap.unbind();
                glViewport(0, 0, size.x, size.y);
            }

            if (drawBezierCurve)
                attachedCamera.curve().draw(viewProjection);

            if (cameraRig.freeCameraEnabled())
                cameraIcon.draw(viewProjection, attachedCamera.viewMatrix());

            if (showMinimap)
                minimap.drawOverlay(size.x, size.y);

            window.swapBuffers();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
    return 0;
}
