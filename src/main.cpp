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

#include "free_camera.h"
#include "model.h"
#include "attached_camera.h"
#include "bezier_curve.h"

glm::vec3 controlPoints[] = {
    glm::vec3(-3.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 3.0f, 0.0f),
    glm::vec3( 1.0f, 3.0f, 0.0f),
    glm::vec3( 3.0f, 0.0f, 0.0f),
    glm::vec3(4.0f, 5.0f, 0.0f),
    glm::vec3(5.0f, -5.0f, 0.0f),
    glm::vec3(6.0f, 0.0f, 0.0f)
};

int main()
{
    Window window(
        "Cable Car Through the Alps",
        glm::ivec2(1280, 720),
        OpenGLVersion::GL41
    );

    AttachedCamera attachedCamera(window, glm::vec3(0.0f, 0.0f, 0.0f));
    FreeCamera freeCamera(&window, glm::vec3(1.5f, 1.0f, 1.0f), -glm::vec3(1.5f, 1.0f, 1.0f));


    glEnable(GL_DEPTH_TEST);

    try
    {
        const std::string root = PROJECT_ROOT;

        Model cableCar;

        if (!cableCar.load(root + "/assets/cable_car/scene.gltf"))
            throw std::runtime_error("Could not load cable-car model");

        BezierCurve curves[] = {
            BezierCurve(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]),
            BezierCurve(controlPoints[3], controlPoints[4], controlPoints[5], controlPoints[6])
        };

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

        while (!window.shouldClose())
        {
            window.updateInput();
            if (window.isKeyPressed(GLFW_KEY_ESCAPE))
                window.close();

            attachedCamera.updateInput();

            auto size = window.getFrameBufferSize();
            glViewport(0, 0, size.x, size.y);

            glClearColor(0.25f, 0.45f, 0.65f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ImGui::Begin("Camera View Points Controls");
            int mode = attachedCamera.mode();
            if (ImGui::Combo("Camera Mode", &mode, AttachedCamera::MODES, AttachedCamera::MODE_COUNT))
                attachedCamera.setMode(mode);

            ImGui::Separator();
            ImGui::Text("Orbit Camera Settings");
            float yaw = attachedCamera.yaw();
            float pitch = attachedCamera.pitch();
            float distance = attachedCamera.distance();
            if (ImGui::SliderFloat("Yaw", &yaw, AttachedCamera::MIN_YAW, AttachedCamera::MAX_YAW))
                attachedCamera.setYaw(yaw);
            if (ImGui::SliderFloat("Pitch", &pitch, AttachedCamera::MIN_PITCH, AttachedCamera::MAX_PITCH))
                attachedCamera.setPitch(pitch);
            if (ImGui::SliderFloat("Distance", &distance, AttachedCamera::MIN_DISTANCE, AttachedCamera::MAX_DISTANCE))
                attachedCamera.setDistance(distance);
            ImGui::End();

            glm::mat4 projection = glm::perspective(
                glm::radians(45.0f),
                1280.0f / 720.0f,
                0.1f,
                1000.0f
            );
            const glm::mat4 view_projection = projection * attachedCamera.viewMatrix();

            // START draw cable car
            basicShader.bind();

            glUniform3f(
               basicShader.getUniformLocation("lightDir"),
                -0.5f, -1.0f, -0.3f
            );
            glUniformMatrix4fv(
                basicShader.getUniformLocation("viewProjection"),
                1,
                GL_FALSE,
                glm::value_ptr(view_projection)
            );

            glUniform3fv(
                basicShader.getUniformLocation("viewPos"),
                1,
                glm::value_ptr(attachedCamera.position())
            );

            cableCar.draw(basicShader, basicShader.getUniformLocation("model"));
            // END draw cable car

            // START draw Bezier curve
            curveShader.bind();
            glUniformMatrix4fv(
                curveShader.getUniformLocation("mvp"),
                1, GL_FALSE, glm::value_ptr(view_projection)
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
