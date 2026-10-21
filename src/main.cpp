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
#include <ostream>
#include <string>
#include <stdexcept>
#include <cmath>

#include "model.h"
#include "bezier_curve.h"

float cameraYaw = 25.0f;
float cameraPitch = 15.0f;
float cameraDistance = 7.0f;

float lastMouseX = 0.0f;
float lastMouseY = 0.0f;

bool firstMouse = true;
bool dragging = false;

int cameraMode = 0; // go through project-files/camera-view-points.txt for better understanding

glm::vec3 controlPoints[] = {
    glm::vec3(-3.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 3.0f, 0.0f),
    glm::vec3( 1.0f, 3.0f, 0.0f),
    glm::vec3( 3.0f, 0.0f, 0.0f),
    glm::vec3(4.0f, 5.0f, 0.0f),
    glm::vec3(5.0f, -5.0f, 0.0f),
    glm::vec3(6.0f, 0.0f, 0.0f)
};

void mouseButtonCallback(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        dragging = (action == GLFW_PRESS);
        firstMouse = true;
    }
}

void mouseMoveCallback(const glm::vec2& position)
{
    if (!dragging)
        return;

    if (firstMouse)
    {
        lastMouseX = position.x;
        lastMouseY = position.y;
        firstMouse = false;
        return;
    }
    if (!dragging)
        return;

    if (firstMouse) {
        lastMouseX = position.x;
        lastMouseY = position.y;
        firstMouse = false;
        return;
    }

    float dx = position.x - lastMouseX;
    float dy = position.y - lastMouseY;

    cameraYaw += dx * 0.3f;
    cameraPitch += dy * 0.3f;

    cameraPitch = glm::clamp(cameraPitch, -85.0f, 85.0f);

    lastMouseX = position.x;
    lastMouseY = position.y;
}

void scrollCallback(const glm::vec2& offset)
{
    cameraDistance -= offset.y * 0.5f;
    cameraDistance = glm::clamp(cameraDistance, 2.0f, 30.0f);
}

int main()
{
    Window window(
        "Cable Car Through the Alps",
        glm::ivec2(1280, 720),
        OpenGLVersion::GL41
    );

    window.registerMouseButtonCallback(mouseButtonCallback);
    window.registerMouseMoveCallback(mouseMoveCallback);
    window.registerScrollCallback(scrollCallback);

    glEnable(GL_DEPTH_TEST);

    try
    {
        const std::string root = PROJECT_ROOT;

        Model cableCar;

        if (!cableCar.load(root + "/assets/cable_car/scene.gltf"))
            throw std::runtime_error("Could not load cable-car model");

        Shader basicShader, curveShader;
        try {
            ShaderBuilder basicBuilder;
            basicBuilder.addStage(GL_VERTEX_SHADER, root + "/shaders/basic.vert");
            basicBuilder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/basic.frag");
            basicShader = basicBuilder.build();

            ShaderBuilder curveBuilder;
            curveBuilder.addStage(GL_VERTEX_SHADER, root + "/shaders/curve.vert");
            curveBuilder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/curve.frag");
            curveShader = curveBuilder.build();
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }

        glm::vec3 cameraPosition(0.0f, 1.5f, 7.0f);

        glm::mat4 view = glm::lookAt(
            cameraPosition,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            1280.0f / 720.0f,
            0.1f,
            1000.0f
        );

        basicShader.bind();

        glUniformMatrix4fv(
            basicShader.getUniformLocation("projection"),
            1, GL_FALSE, glm::value_ptr(projection)
        );

        glUniform3f(
           basicShader.getUniformLocation("lightDir"),
            -0.5f, -1.0f, -0.3f
        );

        BezierCurve curves[] = {
            BezierCurve(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]),
            BezierCurve(controlPoints[3], controlPoints[4], controlPoints[5], controlPoints[6])
        };

        while (!window.shouldClose())
        {
            window.updateInput();
            if (window.isKeyPressed(GLFW_KEY_ESCAPE))
                window.close();

            if (!window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || ImGui::GetIO().WantCaptureMouse) {
                dragging = false;
                firstMouse = true;
            }

            auto size = window.getFrameBufferSize();
            glViewport(0, 0, size.x, size.y);

            glClearColor(0.25f, 0.45f, 0.65f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ImGui::Begin("Camera View Points Controls");

            const char* cameraModes[] = {
                "Orbit View",
                "Top View",
                "Follow View"
            };

            ImGui::Combo(
                "Camera Mode",
                &cameraMode,
                cameraModes,
                IM_ARRAYSIZE(cameraModes)
            );

            ImGui::Separator();
            ImGui::Text("Orbit Camera Settings");

            ImGui::SliderFloat(
                "Yaw",
                &cameraYaw,
                -180.0f,
                180.0f
            );

            ImGui::SliderFloat(
                "Pitch",
                &cameraPitch,
                -85.0f,
                85.0f
            );

            ImGui::SliderFloat(
                "Distance",
                &cameraDistance,
                2.0f,
                30.0f
            );

            ImGui::End();

            glm::vec3 cameraPosition;
            glm::vec3 cameraTarget(0.0f);

            switch (cameraMode)
            {
                case 0:
                {
                    float yaw = glm::radians(cameraYaw);
                    float pitch = glm::radians(cameraPitch);

                    cameraPosition = glm::vec3(
                        cameraDistance * cos(pitch) * sin(yaw),
                        cameraDistance * sin(pitch),
                        cameraDistance * cos(pitch) * cos(yaw)
                    );
                    break;
                }

                case 1:
                {
                    cameraPosition = glm::vec3(0.0f, 12.0f, 0.01f);
                    break;
                }

                case 2:
                {
                    cameraPosition = glm::vec3(0.0f, 2.5f, 8.0f);
                    break;
                }

                default:
                {
                    cameraMode = 0;
                    cameraPosition = glm::vec3(0.0f, 0.0f, cameraDistance);
                    break;
                }
            }

            glm::mat4 view = glm::lookAt(
                cameraPosition,
                cameraTarget,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            // START draw cable car
            basicShader.bind();
            glUniformMatrix4fv(
                basicShader.getUniformLocation("view"),
                1,
                GL_FALSE,
                glm::value_ptr(view)
            );

            glUniform3fv(
                basicShader.getUniformLocation("viewPos"),
                1,
                glm::value_ptr(cameraPosition)
            );

            cableCar.draw(basicShader, basicShader.getUniformLocation("model"));
            // END draw cable car

            // START draw Bezier curve
            curveShader.bind();
            glUniformMatrix4fv(
                curveShader.getUniformLocation("mvp"),
                1, GL_FALSE, glm::value_ptr(projection * view)
            );

            for (const BezierCurve& curve : curves) {
                curve.draw();
            }
            // END draw bezier curve

            window.swapBuffers();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
    return 0;
}
