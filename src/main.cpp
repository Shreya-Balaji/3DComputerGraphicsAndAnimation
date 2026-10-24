#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <framework/window.h>
#include <imgui/imgui.h>
#include <framework/shader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>

#include "cable_car.h"
#include "camera_rig.h"
#include "camera_icon.h"

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
    float doorOpenness = 0.0f;

    glEnable(GL_DEPTH_TEST);

    try
    {
        const std::string root = PROJECT_ROOT;

        CableCar cableCar(doorOpenness);

        CameraIcon cameraIcon;

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
            if (ImGui::SliderFloat("Open doors", &doorOpenness, 0.0f, 1.0f))
                cableCar.setDoorOpenness(doorOpenness);
            if (freeCameraEnabled)
                ImGui::Text("WASD: move | Space/Ctrl: up/down | Left-drag: look");
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

            cableCar.draw(viewProjection, cameraPosition);

           if (drawBezierCurve)
                attachedCamera.curve().draw(viewProjection);

            if (cameraRig.freeCameraEnabled())
                cameraIcon.draw(viewProjection, attachedCamera.viewMatrix());

            window.swapBuffers();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
    return 0;
}
