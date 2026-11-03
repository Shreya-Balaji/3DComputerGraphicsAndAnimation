#include "cable_car.h"
#include "environment.h"
#include "camera_rig.h"
#include "camera_icon.h"
#include "minimap.h"
#include "menu.h"
#include "config.h"
#include "snow_particle_effect.h"
#include "fog.h"

#include "terrain.h"
#include "light.h"
#include "sky.h"
#include "skybox.h"
#include "environment_map.h"
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
        Fog fog(config);

        Minimap minimap;
        Skybox skybox;
        EnvironmentMap environmentMap;

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
            SkyLight light = computeSkyLight(config.timeOfDay);
            const glm::vec3 skyColor = computeSkyColor(light,config);
            glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            float cabinMidY = -1.5f;

            const ImGuiIO& io = ImGui::GetIO();
            cableCar.update(io.DeltaTime);
            light.cabinLightPos = cableCar.position() + glm::vec3(0.0f, cabinMidY, 0.0f);
            if(!config.cabinLightEnabled){
                light.cabinLightAmount = 0.0f;
            }
            cableCarPosition = cableCar.position();
            terrain.update(io.DeltaTime);
            snow.update(io.DeltaTime);
            cameraRig.updateInput(io.WantCaptureMouse || io.WantCaptureKeyboard, io.DeltaTime);
            attachedCameraPosition = attachedCamera.position();

            const glm::mat4 view = cameraRig.viewMatrix();
            const glm::vec3 cameraPosition = cameraRig.position();

            if (size.x <= 0 || size.y <= 0) {
                window.swapBuffers();
                continue;
            }

            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                static_cast<float>(size.x) / size.y,
                0.1f,
                1000.0f
            );
            const glm::mat4 viewProjection = projection * view;
            if (config.useEnvMap) {
                const glm::vec3 capturePos = cableCar.position() + glm::vec3(0.0f, 1.0f, 0.0f);
                environmentMap.capture(capturePos,[&](const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos) {
                    skybox.draw(v, p, pos, light, skyColor);
                    terrain.drawReflection(p * v, light);
                });
            }
            environmentMap.bind(2);

            fog.begin(size.x, size.y);
            glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            skybox.draw(view, projection, cameraPosition, light, skyColor);
            cableCar.draw(viewProjection, cameraPosition, light, config.usePBR);

            // environment.draw(view, projection);
            terrain.draw(viewProjection, cameraPosition, light);

            if (config.drawBezierCurve)
                attachedCamera.curve().draw(viewProjection * glm::translate(glm::mat4(1.0f), cableCar.position()));

            if (config.freeCameraEnabled)
                cameraIcon.draw(viewProjection, attachedCamera.viewMatrix());

            fog.draw(projection);
            snow.draw(viewProjection, cameraPosition);

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
                minimap.drawOverlay(size.x, size.y);
            }

            window.swapBuffers();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
    return 0;
}
