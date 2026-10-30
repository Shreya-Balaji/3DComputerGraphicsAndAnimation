#include "cable_car.h"
#include "environment.h"
#include "camera_rig.h"
#include "camera_icon.h"
#include "minimap.h"
#include "menu.h"
#include "config.h"

#include <framework/window.h>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <exception>
#include <iostream>
#include <stdexcept>

int main()
{
    Window window(
        "Cable Car Through the Alps",
        glm::ivec2(1280, 720),
        OpenGLVersion::GL41
    );

    Config config;

    glEnable(GL_DEPTH_TEST);

    try
    {
        CableCar cableCar(config);
        CameraRig cameraRig(window, cableCar, config);
        const AttachedCamera& attachedCamera = cameraRig.attachedCamera();
        Menu menu(config, attachedCamera.maxTValue());

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
            menu.draw();

            const ImGuiIO& io = ImGui::GetIO();
            cableCar.update(io.DeltaTime);
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

            environment.draw(view, projection);

            if (config.showMinimap)
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

            if (config.drawBezierCurve)
                attachedCamera.curve().draw(viewProjection);

            if (config.freeCameraEnabled)
                cameraIcon.draw(viewProjection, attachedCamera.viewMatrix());

            if (config.showMinimap)
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
