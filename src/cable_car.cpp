#include "cable_car.h"

#include <assimp/material.h>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/common.hpp>
#include <cmath>
#include <iostream>
#include <string>
#include "light.h"
#include "environment_map.h"
#include <glm/geometric.hpp>
#include <glm/gtc/matrix_transform.hpp>

void CableCar::bindLighting(const Shader& shader, const glm::mat4& viewProjection,
                            const glm::vec3& cameraPosition, const SkyLight& light) const
{
    shader.bind();
    glUniform1i(shader.getUniformLocation("useNormalMaps"), m_config.useNormalMaps);
    glUniformMatrix4fv(shader.getUniformLocation("viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform3fv(shader.getUniformLocation("viewPos"), 1, glm::value_ptr(cameraPosition));
    glUniform3fv(shader.getUniformLocation("lightDir"), 1, glm::value_ptr(light.sunDir));
    glUniform3fv(shader.getUniformLocation("sunColor"), 1, glm::value_ptr(light.sunColor));
    glUniform3fv(shader.getUniformLocation("moonDir"), 1, glm::value_ptr(light.moonDir));
    glUniform3fv(shader.getUniformLocation("moonColor"), 1, glm::value_ptr(light.moonColor));
    glUniform3fv(shader.getUniformLocation("ambientColor"), 1, glm::value_ptr(light.ambientColor));
    glUniform3fv(shader.getUniformLocation("cabinLightPos"), 1, glm::value_ptr(light.cabinLightPos));
    glUniform3fv(shader.getUniformLocation("cabinLightColor"), 1, glm::value_ptr(m_config.cabinLightColor));
    glUniform1f(shader.getUniformLocation("cabinLightAmount"), light.cabinLightAmount);
    glUniform1f(shader.getUniformLocation("cabinLightRadius"), m_config.cabinLightRadius);
    glUniform1f(shader.getUniformLocation("cabinLightIntensity"), m_config.cabinLightIntensity);
}
void CableCar::createCable()
{
    // Create a mesh that will contain the procedurally generated cable geometry
    Mesh cable;
    
    // Define Cable geometry and dimensions
    cable.material.kd = glm::vec3(0.12f, 0.12f, 0.14f);
    cable.material.ks = glm::vec3(0.5f);
    cable.material.shininess = 64.0f;
    cable.material.transparency = 1.0f;

    constexpr int segments = 128;
    constexpr int radialSegments = 8;
    constexpr float radius = 0.0095f;
    constexpr float cableLength = 64.0f;


    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) / segments;

        glm::vec3 center(
            -5.4f,
            0.92f,
            -cableLength * 0.5f + t * cableLength
        );

        glm::vec3 tangent(0.0f, 0.0f, 1.0f);
        glm::vec3 referenceUp(0.0f, 1.0f, 0.0f);

        glm::vec3 side = glm::normalize(
            glm::cross(tangent, referenceUp));

        glm::vec3 up = glm::normalize(
            glm::cross(side, tangent));

        for (int j = 0; j < radialSegments; ++j)
        {
            float angle =
                glm::two_pi<float>() * j / radialSegments;

            glm::vec3 normal =
                glm::cos(angle) * side +
                glm::sin(angle) * up;

            Vertex vertex;
            vertex.position = center + radius * normal;
            vertex.normal = normal;
            vertex.texCoord = glm::vec2(
                static_cast<float>(i) / segments,
                static_cast<float>(j) / radialSegments
            );
            vertex.tangent = glm::vec4(tangent, 1.0f);

            cable.vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < segments; ++i)
    {
        for (int j = 0; j < radialSegments; ++j)
        {
            unsigned int current =
                i * radialSegments + j;

            unsigned int next =
                i * radialSegments +
                (j + 1) % radialSegments;

            unsigned int nextRing =
                (i + 1) * radialSegments + j;

            unsigned int nextRingNext =
                (i + 1) * radialSegments +
                (j + 1) % radialSegments;

            cable.triangles.emplace_back(
                current, next, nextRing);

            cable.triangles.emplace_back(
                next, nextRingNext, nextRing);
        }
    }

    // Upload the mesh to GPU so that it can be rendered.
    m_cableMesh = std::make_unique<GPUMesh>(cable);
}

CableCar::CableCar(const Config& config)
: m_config(config)
// Loading the cable car model
, m_model(Model(std::string(PROJECT_ROOT) + "/assets/cable_car/scene.gltf"))
{
    const aiScene* scene = m_model.getScene();
    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        aiString materialName;
        const unsigned int materialIndex = scene->mMeshes[i]->mMaterialIndex;
        if (materialIndex < scene->mNumMaterials)
            scene->mMaterials[materialIndex]->Get(AI_MATKEY_NAME, materialName);
        if (std::string(materialName.C_Str()) == "Windows")
            m_windowMeshes.push_back(i);
        else
            m_bodyMeshes.push_back(i);
    }

    try {
        ShaderBuilder cableCarBuilder;
        cableCarBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        cableCarBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/cable_car.frag");
        m_cableCarShader = cableCarBuilder.build();
        ShaderBuilder pbrBuilder;
        pbrBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        pbrBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/pbr.frag");
        m_pbrShader = pbrBuilder.build();
        ShaderBuilder windowBuilder;
        windowBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        windowBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/window.frag");
        m_windowShader = windowBuilder.build();
        ShaderBuilder depthBuilder;
        depthBuilder.addStage(GL_VERTEX_SHADER, std::string(PROJECT_ROOT) + "/shaders/basic.vert");
        depthBuilder.addStage(GL_FRAGMENT_SHADER, std::string(PROJECT_ROOT) + "/shaders/depth.frag");
        m_depthShader = depthBuilder.build();


    } catch (const ShaderLoadingException& e) {
        std::cerr << e.what() << std::endl;
        throw;
    }
    createCable();
}

glm::vec3 CableCar::position() const
{
    return glm::vec3(-5.4f, 0.26f, -5.9f);
}

void CableCar::update(float deltaTime)
{
    m_model.setWorldTransform(position(), 0.55f);

    m_model.setNodeWorldOffset("Door_Back_Left", -OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Back_Right", -OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Front_Left", OPEN_DOOR * m_config.doorSlideFactor);
    m_model.setNodeWorldOffset("Door_Front_Right", OPEN_DOOR * m_config.doorSlideFactor);

    const glm::vec3 windowOffset(0.0f, m_config.doorWindowsOpen ? -0.12f : 0.0f, 0.0f);
    for (const char* window : {"Door_Back_Left.Window", "Door_Back_Right.Window",
                               "Door_Front_Left.Window", "Door_Front_Right.Window"})
        m_model.setNodeWorldOffset(window, windowOffset);

    m_wheelAngle = std::fmod(
        m_wheelAngle + glm::radians(m_config.wheelSpeed) * deltaTime,
        glm::two_pi<float>());

    for (int i = 1; i <= 8; ++i)
        m_model.setNodeRotation(
            "Wheel_" + std::to_string(i),
            m_wheelAngle,
            glm::vec3(1.0f, 0.0f, 0.0f));
}

void CableCar::draw(const glm::mat4& viewProjection, const glm::vec3& cameraPosition, const SkyLight& light) const
{
    // Render the cable using the normal world-space transformation.
    drawCables(viewProjection, cameraPosition, light, glm::mat4(1.0f));
    drawBody(viewProjection, cameraPosition, light);
}

void CableCar::drawMinimap(const glm::mat4& viewProjection, const glm::vec3& cameraPosition, const SkyLight& light) const
{
    // Construct a transformation for displaying the cable in minimap.
    glm::mat4 cableTransform = glm::translate(glm::mat4(1.0f), glm::vec3(-5.4f, 0.92f, 0.0f));
    cableTransform = glm::scale(cableTransform, glm::vec3(5.0f, 5.0f, 1.0f));
    cableTransform = glm::translate(cableTransform, glm::vec3(5.4f, -0.92f, 0.0f));
    drawCables(viewProjection, cameraPosition, light, cableTransform);
    drawBody(viewProjection, cameraPosition, light);
}

// Code for Cable mesh
void CableCar::drawCables(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                          const SkyLight& light, const glm::mat4& cableTransform) const
{
    if (!m_cableMesh)
        return;

    m_cableCarShader.bind();
    glUniform1i(m_cableCarShader.getUniformLocation("useNormalMaps"), m_config.useNormalMaps);
    glUniform1i(m_cableCarShader.getUniformLocation("shadowMap"), 3);

    glUniformMatrix4fv(
        m_cableCarShader.getUniformLocation("viewProjection"),
        1,
        GL_FALSE,
        glm::value_ptr(viewProjection)
    );

    glUniform3fv(
        m_cableCarShader.getUniformLocation("viewPos"),
        1,
        glm::value_ptr(cameraPosition)
    );

    glUniform3fv(m_cableCarShader.getUniformLocation("lightDir"), 1,
        glm::value_ptr(light.sunDir));

    glUniform3fv(m_cableCarShader.getUniformLocation("sunColor"), 1,
        glm::value_ptr(light.sunColor));

    glUniform3fv(m_cableCarShader.getUniformLocation("moonDir"), 1,
        glm::value_ptr(light.moonDir));

    glUniform3fv(m_cableCarShader.getUniformLocation("moonColor"), 1,
        glm::value_ptr(light.moonColor));

    glUniform3fv(m_cableCarShader.getUniformLocation("ambientColor"), 1,
        glm::value_ptr(light.ambientColor));

    constexpr float cableTileSize = 64.0f;

    for (int z = -1; z <= 1; ++z)
    {
        glm::mat4 cableModel = glm::translate(
            glm::mat4(1.0f),
            glm::vec3(
                0.0f,
                0.0f,
                z * cableTileSize
            )
        );
        cableModel *= cableTransform;

        glUniformMatrix4fv(m_cableCarShader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(cableModel));
        m_cableMesh->draw(m_cableCarShader);
    }
}

// Code for cable car body
void CableCar::drawBody(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                        const SkyLight& light) const
{
    if (m_bodyMeshes.empty())
        return;

    const Shader& shader = m_config.usePBR ? m_pbrShader : m_cableCarShader;
    bindLighting(shader, viewProjection, cameraPosition, light);
    if (m_config.usePBR) {
        glUniform1f(shader.getUniformLocation("metallic"), m_config.pbrMetallic);
        glUniform1f(shader.getUniformLocation("roughness"), m_config.pbrRoughness);
    }
    else {
        glUniform1i(shader.getUniformLocation("shadowMap"), 3);

        glUniform1i(
            shader.getUniformLocation("ambientLightingEnabled"),
            m_config.ambientLightingEnabled ? 1 : 0
        );

        glUniform1f(
            shader.getUniformLocation("shininessOverride"),
            m_config.shininess
        );
    }
    m_model.draw(shader, m_bodyMeshes);
}
// Code for cable car windows.
void CableCar::drawWindows(const glm::mat4& viewProjection, const glm::vec3& cameraPosition,
                           const SkyLight& light) const
{
    if (m_windowMeshes.empty() || m_config.windowOpacity <= 0.0f)
        return;

    // For blending the environment map with the transparent window
    const GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLint sourceRGB, destinationRGB, sourceAlpha, destinationAlpha;
    glGetIntegerv(GL_BLEND_SRC_RGB, &sourceRGB);
    glGetIntegerv(GL_BLEND_DST_RGB, &destinationRGB);
    glGetIntegerv(GL_BLEND_SRC_ALPHA, &sourceAlpha);
    glGetIntegerv(GL_BLEND_DST_ALPHA, &destinationAlpha);
    glEnable(GL_BLEND);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    bindLighting(m_windowShader, viewProjection, cameraPosition, light);
    glUniform1f(m_windowShader.getUniformLocation("windowOpacity"), m_config.windowOpacity);
    glUniform1i(m_windowShader.getUniformLocation("envMap"), 2);
    glUniform1f(m_windowShader.getUniformLocation("envMaxLod"), EnvironmentMap::MAX_LOD);
    glUniform1i(m_windowShader.getUniformLocation("useEnvMap"), m_config.useEnvMap);
    glUniform1i(m_windowShader.getUniformLocation("windowPCFSampling"), m_config.windowPCFSampling);
    m_model.draw(m_windowShader, m_windowMeshes);

    glBlendFuncSeparate(sourceRGB, destinationRGB, sourceAlpha, destinationAlpha);
    if (!blendEnabled)
        glDisable(GL_BLEND);
}
void CableCar::drawDepth(const glm::mat4& lightCamera) const
{
    m_depthShader.bind();
    glUniformMatrix4fv(m_depthShader.getUniformLocation("viewProjection"),
                       1, GL_FALSE, glm::value_ptr(lightCamera));
    m_model.draw(m_depthShader);
}
