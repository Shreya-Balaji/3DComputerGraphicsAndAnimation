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
#include "skybox.h"
#include "environment_map.h"
#include <framework/window.h>
#include <imgui/imgui.h>
#include <glm/gtc/matrix_transform.hpp>
#include <exception>
#include <iostream>
#include <stdexcept>
#include "shadow.h"
#include <cmath>
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
        Skybox skybox;
        EnvironmentMap environmentMap;
        ShadowMap shadowMap;
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
            light.cabinLightPos = cableCar.position() + glm::vec3(0.0f, cabinMidY, 0.0f);
            if(!config.cabinLightEnabled){
                light.cabinLightAmount = 0.0f;
            }
            terrain.update(io.DeltaTime);
            cableCarPosition = cableCar.position();
            cableCar.update(io.DeltaTime, terrain.scrollOffset());
            cableCarPosition = cableCar.position();
            snow.update(io.DeltaTime);
            cameraRig.updateInput(io.WantCaptureMouse || io.WantCaptureKeyboard, io.DeltaTime);
            attachedCameraPosition = cameraRig.position();

            const glm::mat4 view = cameraRig.viewMatrix();
            const glm::vec3 cameraPosition = cameraRig.position();

            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                1280.0f / 720.0f,
                0.1f,
                1000.0f
            );
            const glm::mat4 viewProjection = projection * view;
            glm::vec3 shadowLightDir = light.sunDir.y < 0 ? light.sunDir : light.moonDir;
            light.lightCamera = computeLightSpace(shadowLightDir, cableCar.position());
            shadowMap.begin();
            cableCar.drawDepth(light.lightCamera);
            shadowMap.end();
            shadowMap.bind(3);
            const glm::vec3 capturePos = cableCar.position() + glm::vec3(0.0f, 1.0f, 0.0f);
            environmentMap.capture(capturePos,[&](const glm::mat4& v, const glm::mat4& p, const glm::vec3& pos) {
                skybox.draw(v, p, pos, light, skyColor);
                terrain.draw(p * v, pos, light);
            });
            environmentMap.bind(2);
            skybox.draw(view, projection, cameraPosition, light, skyColor);
            
            cableCar.draw(viewProjection, cameraPosition, light, config.usePBR, false);

            terrain.draw(viewProjection, cameraPosition, light);

            if (config.showMinimap)
            {
                minimap.bind();

                glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                const glm::vec3 currentCableCarPosition = cableCar.position();

                const glm::vec3 minimapCameraPosition(
                    currentCableCarPosition.x,
                    currentCableCarPosition.y + 70.0f,
                    currentCableCarPosition.z
                );

                const glm::mat4 minimapView = glm::lookAt(
                    minimapCameraPosition,
                    currentCableCarPosition,
                    glm::vec3(0.0f, 0.0f, -1.0f)
                );

                const glm::mat4 minimapProjection = minimap.projectionMatrix();

                const glm::mat4 minimapViewProjection =
                    minimapProjection * minimapView;

                terrain.draw(minimapViewProjection, minimapCameraPosition, light);

                glDisable(GL_DEPTH_TEST);

                cableCar.draw(minimapViewProjection, minimapCameraPosition, light, config.usePBR, true);

                glEnable(GL_DEPTH_TEST);

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
