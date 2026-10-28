
#include "environment.h"
#include "model.h"

#include <glm/gtc/type_ptr.hpp>

#include <exception>
#include <iostream>
#include <memory>
#include <string>

Environment::~Environment() = default;

bool Environment::initialize()
{
    try
    {
        const std::string root = PROJECT_ROOT;

        const std::string mountainPath =
            root + "/assets/mountain_range_01/scene.gltf";

        mountain = std::make_unique<Model>(mountainPath);
        mountain->setWorldTransform(
            glm::vec3(-3.0f, -2.0f, -5.0f),
            3.0f,
            glm::vec3(0.0f, 0.0f, 0.0f)
        );

        ShaderBuilder builder;

        builder.addStage(
            GL_VERTEX_SHADER,
            root + "/shaders/basic.vert"
        );

        builder.addStage(
            GL_FRAGMENT_SHADER,
            root + "/shaders/basic.frag"
        );

        environmentShader = builder.build();

        std::cout << "Mountain environment initialized.\n";
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to initialize environment: "
            << e.what() << '\n';
        return false;
    }
}


void Environment::draw(
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::vec3& cameraPosition,
    const glm::vec3& sunLightDir
)
{
    if (!mountain)
        return;

    environmentShader.bind();

    const glm::mat4 viewProjection = projection * view;

    glUniformMatrix4fv(
        environmentShader.getUniformLocation("viewProjection"),
        1,
        GL_FALSE,
        glm::value_ptr(viewProjection)
    );

    glUniform3f(
        environmentShader.getUniformLocation("lightDir"),
        -0.5f, -1.0f, -0.3f
    );
    glUniform3fv(
        environmentShader.getUniformLocation("viewPos"),
        1,
        glm::value_ptr(cameraPosition)
    );
    glUniform3fv(
        environmentShader.getUniformLocation("lightDir"),
        1,
        glm::value_ptr(sunLightDir)
    );

    mountain->draw(environmentShader);
}
