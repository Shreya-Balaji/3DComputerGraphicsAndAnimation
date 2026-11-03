#include "terrain.h"

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <string>
#include "light.h"

namespace {
constexpr int CELLS = 32;
constexpr int SIMPLIFIED_CELLS = 16;
constexpr float TILE_SIZE = 64.0f;
constexpr float TEXTURE_REPEATS = 4.0f;

Vertex terrainVertex(int x, int z, int cells)
{
    // Ensure that edges have the same parameters
    const float a = glm::two_pi<float>() * static_cast<float>(x % cells) / cells;
    const float b = glm::two_pi<float>() * static_cast<float>(z % cells) / cells;
    const float ridge = std::pow(std::abs(std::cos(a * 0.5f)), 1.5f);
    const float leftPeaks = 14.0f + 5.0f * std::sin(b + 0.6f)
        + 3.0f * std::cos(3.0f * b - 1.1f) + 1.5f * std::sin(5.0f * b + 0.8f);
    const float rightPeaks = 16.0f + 4.0f * std::sin(2.0f * b + 1.7f)
        + 3.0f * std::cos(3.0f * b + 0.4f) + 2.0f * std::sin(5.0f * b - 1.2f);
    const float peaks = glm::mix(leftPeaks, rightPeaks, static_cast<float>(x) / cells)
        + 1.5f * std::sin(3.0f * b + 2.0f * a);

    Vertex vertex{};
    vertex.position = glm::vec3(
        (static_cast<float>(x) / cells - 0.5f) * TILE_SIZE,
        -10.0f + ridge * peaks + 0.6f * std::sin(2.0f * b),
        (static_cast<float>(z) / cells - 0.5f) * TILE_SIZE);
    vertex.texCoord = glm::vec2(x, z) / static_cast<float>(cells) * TEXTURE_REPEATS;
    return vertex;
}

void addTriangle(Mesh& mesh, Vertex a, Vertex b, Vertex c)
{
    const glm::vec3 edge1 = b.position - a.position;
    const glm::vec3 edge2 = c.position - a.position;
    const glm::vec2 uv1 = b.texCoord - a.texCoord;
    const glm::vec2 uv2 = c.texCoord - a.texCoord;
    const float determinant = uv1.x * uv2.y - uv1.y * uv2.x;
    const glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));
    const glm::vec3 tangent = glm::normalize((edge1 * uv2.y - edge2 * uv1.y) / determinant);
    const glm::vec3 bitangent = (edge2 * uv1.x - edge1 * uv2.x) / determinant;
    const float handedness = glm::dot(glm::cross(normal, tangent), bitangent) < 0.0f ? -1.0f : 1.0f;

    const auto first = static_cast<unsigned int>(mesh.vertices.size());
    for (auto vertex : {a, b, c}) {
        vertex.normal = normal;
        vertex.tangent = glm::vec4(tangent, handedness);
        mesh.vertices.push_back(vertex);
    }
    mesh.triangles.emplace_back(first, first + 1, first + 2);
}

Mesh terrainMesh(int cells)
{
    Mesh mesh;
    mesh.vertices.reserve(cells * cells * 6);
    mesh.triangles.reserve(cells * cells * 2);
    for (int z = 0; z < cells; ++z) {
        for (int x = 0; x < cells; ++x) {
            const auto a = terrainVertex(x, z, cells);
            const auto b = terrainVertex(x + 1, z, cells);
            const auto c = terrainVertex(x, z + 1, cells);
            const auto d = terrainVertex(x + 1, z + 1, cells);
            if ((x + z) % 2 == 0) {
                addTriangle(mesh, a, c, b);
                addTriangle(mesh, b, c, d);
            } else {
                addTriangle(mesh, a, c, d);
                addTriangle(mesh, a, d, b);
            }
        }
    }
    return mesh;
}
}

Terrain::Terrain(const Config& config, const glm::vec3& position)
    : m_config(config)
    , m_position(position)
{
    const std::string root = PROJECT_ROOT;
    ShaderBuilder builder;
    builder.addStage(GL_VERTEX_SHADER, root + "/shaders/basic.vert");
    builder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/basic.frag");
    m_shader = builder.build();

    ShaderBuilder reflectionBuilder;
    reflectionBuilder.addStage(GL_VERTEX_SHADER, root + "/shaders/terrain_reflection.vert");
    reflectionBuilder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/terrain_reflection.frag");
    m_reflectionShader = reflectionBuilder.build();
    Mesh reflectionMesh = terrainMesh(SIMPLIFIED_CELLS);
    m_reflectionMesh = std::make_unique<GPUMesh>(reflectionMesh);
    Mesh mesh = terrainMesh(CELLS);
    mesh.material.kd = glm::vec3(1.0f);
    mesh.material.ks = glm::vec3(0.04f);
    mesh.material.shininess = 16.0f;
    mesh.material.kdTexture = std::make_shared<Image>(root + "/assets/snow_02_diff_1k.png");
    mesh.material.normalTexture = std::make_shared<Image>(root + "/assets/snow_02_nor_gl_1k.png");
    m_mesh = std::make_unique<GPUMesh>(mesh);
}

void Terrain::drawReflection(const glm::mat4& viewProjection, const SkyLight& light) const
{
    if (!m_config.terrainEnabled)
        return;

    m_reflectionShader.bind();
    glUniformMatrix4fv(m_reflectionShader.getUniformLocation("viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform3fv(m_reflectionShader.getUniformLocation("sunDir"), 1, glm::value_ptr(light.sunDir));
    glUniform3fv(m_reflectionShader.getUniformLocation("sunColor"), 1, glm::value_ptr(light.sunColor));
    glUniform3fv(m_reflectionShader.getUniformLocation("moonDir"), 1, glm::value_ptr(light.moonDir));
    glUniform3fv(m_reflectionShader.getUniformLocation("moonColor"), 1, glm::value_ptr(light.moonColor));
    glUniform3fv(m_reflectionShader.getUniformLocation("ambientColor"), 1, glm::value_ptr(light.ambientColor));

    const GLint modelLocation = m_reflectionShader.getUniformLocation("model");
    for (int z = -1; z <= 1; ++z) {
        const auto model = glm::translate(glm::mat4(1.0f),
            m_position + glm::vec3(0.0f, 0.0f, z * TILE_SIZE + static_cast<float>(m_scroll)));
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
        m_reflectionMesh->drawGeometry();
    }
}

void Terrain::update(float deltaTime)
{
    if (m_config.terrainEnabled)
        m_scroll = std::remainder(m_scroll + static_cast<double>(m_config.terrainSpeed) * deltaTime, TILE_SIZE);
}

void Terrain::draw(const glm::mat4& viewProjection, const glm::vec3& viewPosition, const SkyLight& light) const
{
    if (!m_config.terrainEnabled)
        return;

    m_shader.bind();
    glUniform1i(m_shader.getUniformLocation("twoScaleSampling"), m_config.terrainTwoScaleSampling);
    glUniformMatrix4fv(m_shader.getUniformLocation("viewProjection"), 1, GL_FALSE, glm::value_ptr(viewProjection));
    glUniform3fv(m_shader.getUniformLocation("viewPos"), 1, glm::value_ptr(viewPosition));
    glUniform3fv(m_shader.getUniformLocation("lightDir"), 1, glm::value_ptr(light.sunDir));
    glUniform3fv(m_shader.getUniformLocation("sunColor"), 1, glm::value_ptr(light.sunColor));
    glUniform3fv(m_shader.getUniformLocation("moonDir"), 1, glm::value_ptr(light.moonDir));
    glUniform3fv(m_shader.getUniformLocation("moonColor"), 1, glm::value_ptr(light.moonColor));
    glUniform3fv(m_shader.getUniformLocation("ambientColor"), 1, glm::value_ptr(light.ambientColor));
    glUniform3fv(m_shader.getUniformLocation("cabinLightPos"), 1, glm::value_ptr(light.cabinLightPos));
    glUniform3fv(m_shader.getUniformLocation("cabinLightColor"), 1, glm::value_ptr(m_config.cabinLightColor));
    glUniform1f(m_shader.getUniformLocation("cabinLightAmount"), light.cabinLightAmount);
    glUniform1f(m_shader.getUniformLocation("cabinLightRadius"), m_config.cabinLightRadius);
    glUniform1f(m_shader.getUniformLocation("cabinLightIntensity"), m_config.cabinLightIntensity);

    for (int z = -1; z <= 1; ++z) {
        const auto model = glm::translate(glm::mat4(1.0f),
            m_position + glm::vec3(0.0f, 0.0f, z * TILE_SIZE + static_cast<float>(m_scroll)));
        glUniformMatrix4fv(m_shader.getUniformLocation("model"), 1, GL_FALSE, glm::value_ptr(model));
        m_mesh->draw(m_shader);
    }
}
