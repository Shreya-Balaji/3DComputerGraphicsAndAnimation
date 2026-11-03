#include "cable_car.h"
#include "environment.h"
#include "camera_rig.h"
#include "camera_icon.h"
#include "minimap.h"
#include "menu.h"
#include "config.h"
#include "snow_particle_effect.h"
#include "terrain.h"
#include "light.h"
#include "sky.h"
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
        glm::vec3 cableCarPosition = cableCar.position();
        Terrain terrain(config, cableCarPosition);
        CameraRig cameraRig(window, cableCar, config);
        const AttachedCamera& attachedCamera = cameraRig.attachedCamera();
        glm::vec3 attachedCameraPosition = attachedCamera.position();
        Menu menu(config, attachedCamera.curve().length());

        // Temporarily disabled to avoid loading the mountain asset.
        /* Environment environment;
        if (!environment.initialize())
        {
            throw std::runtime_error(
                "Could not initialize mountain environment."
            );
        } */

        CameraIcon cameraIcon;
        SnowParticleEffect snow(config, attachedCameraPosition);

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
            menu.draw();
            const SkyLight light = computeSkyLight(config.timeOfDay);
            const glm::vec3 skyColor = computeSkyColor(light,config);
            glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            

            const ImGuiIO& io = ImGui::GetIO();
            cableCar.update(io.DeltaTime);
            cableCarPosition = cableCar.position();
            terrain.update(io.DeltaTime);
            snow.update(io.DeltaTime);
            cameraRig.updateInput(io.WantCaptureMouse || io.WantCaptureKeyboard, io.DeltaTime);
            attachedCameraPosition = attachedCamera.position();

            const glm::mat4 view = cameraRig.viewMatrix();
            const glm::vec3 cameraPosition = cameraRig.position();

            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                1280.0f / 720.0f,
                0.1f,
                1000.0f
            );
            const glm::mat4 viewProjection = projection * view;

            cableCar.draw(viewProjection, cameraPosition, light, config.usePBR);

            terrain.draw(viewProjection, cameraPosition, light);

            if (config.showMinimap)
            {
                minimap.bind();

                glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                const glm::mat4 minimapView = minimap.viewMatrix();
                const glm::mat4 minimapProjection = minimap.projectionMatrix();
                const glm::mat4 minimapViewProjection =
                    minimapProjection * minimapView;

                const glm::vec3 minimapCameraPosition(-3.0f, 70.0f, -5.0f);

                terrain.draw(minimapViewProjection, minimapCameraPosition, light);
                cableCar.draw(minimapViewProjection, minimapCameraPosition, light, config.usePBR);

                minimap.unbind();
                glViewport(0, 0, size.x, size.y);
            }

            if (config.drawBezierCurve)
                attachedCamera.curve().draw(viewProjection * glm::translate(glm::mat4(1.0f), cableCar.position()));

            if (config.freeCameraEnabled)
                cameraIcon.draw(viewProjection, attachedCamera.viewMatrix());

            snow.draw(viewProjection);

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
