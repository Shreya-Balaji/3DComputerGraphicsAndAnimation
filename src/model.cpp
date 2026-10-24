#include "model.h"
#include "assimp_utils.h"

#include <assimp/postprocess.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdexcept>

void Model::calculateBounds(
    const aiNode* node,
    const aiMatrix4x4& parentTransform,
    glm::vec3& minimum,
    glm::vec3& maximum,
    bool& firstVertex) const
{
    aiMatrix4x4 transform =
        parentTransform * node->mTransformation;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh =
            m_scene->mMeshes[node->mMeshes[i]];

        for (unsigned int j = 0; j < mesh->mNumVertices; ++j)
        {
            aiVector3D p = transform * mesh->mVertices[j];
            glm::vec3 v(p.x, p.y, p.z);

            if (firstVertex)
            {
                minimum = maximum = v;
                firstVertex = false;
            }
            else
            {
                minimum = glm::min(minimum, v);
                maximum = glm::max(maximum, v);
            }
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i)
    {
        calculateBounds(
            node->mChildren[i],
            transform,
            minimum,
            maximum,
            firstVertex
        );
    }
}

Model::Model(const std::string& path) {
    load(path);
}

void Model::load(const std::string& path)
{
    m_scene = m_importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality
    );

    if (!m_scene || !m_scene->mRootNode)
    {
        throw std::runtime_error("Could not load model: " + path);
    }

    m_meshes.clear();
    m_meshes.reserve(m_scene->mNumMeshes);

    for (unsigned int i = 0; i < m_scene->mNumMeshes; ++i) {
        const aiMesh& source = *m_scene->mMeshes[i];
        Mesh cpuMesh = toCPUMesh(source, *m_scene);

        std::cout << "Mesh " << i << '\n';

        std::cout << "kd: "
            << cpuMesh.material.kd.r << ", "
            << cpuMesh.material.kd.g << ", "
            << cpuMesh.material.kd.b << '\n';

        std::cout << "ks: "
            << cpuMesh.material.ks.r << ", "
            << cpuMesh.material.ks.g << ", "
            << cpuMesh.material.ks.b << '\n';

        std::cout << "shininess: "
            << cpuMesh.material.shininess << '\n';

        m_meshes.emplace_back(cpuMesh);
    }

    // Calculate the complete model bounds, including node transforms.
    bool firstVertex = true;
    aiMatrix4x4 identity;

    calculateBounds(
        m_scene->mRootNode,
        identity,
        m_boundsMin,
        m_boundsMax,
        firstVertex
    );

    if (firstVertex)
    {
        throw std::runtime_error("Model contains no vertices.\n");
    }

    m_modelCenter = (m_boundsMin + m_boundsMax) * 0.5f;

    glm::vec3 dimensions = m_boundsMax - m_boundsMin;

    float maxDimension = glm::max(
        dimensions.x,
        glm::max(dimensions.y, dimensions.z)
    );

    if (maxDimension <= 0.000001f)
    {
        throw std::runtime_error("Invalid model dimensions.\n");
    }

    // Fit the largest dimension to approximately 4 world units.
    m_modelScale = 4.0f / maxDimension;

    std::cout << "Model loaded successfully!\n";
    std::cout << "Meshes: " << m_scene->mNumMeshes << '\n';
    std::cout << "Materials: " << m_scene->mNumMaterials << '\n';

    std::cout << "Model dimensions: "
        << dimensions.x << ", "
        << dimensions.y << ", "
        << dimensions.z << '\n';

    std::cout << "Model center: "
        << m_modelCenter.x << ", "
        << m_modelCenter.y << ", "
        << m_modelCenter.z << '\n';

    std::cout << "Automatic scale: "
        << m_modelScale << '\n';
}

// function to set an offset for child nodes of the model, the transform will be applied in the drawNode function
bool Model::setNodeWorldOffset(const std::string &name, const glm::vec3 offset) {
  const aiNode* node = getNode(name);

  if (!node)
    return false;

  m_nodeWorldOffsets[node] = offset;
  return true;
};

void Model::drawNode(
    const aiNode* node,
    const glm::mat4& parentTransform,
    const Shader& shader) const
{
    glm::mat4 transform =
        parentTransform * toGLM(node->mTransformation);

    auto offset = m_nodeWorldOffsets.find(node);

    if (offset != m_nodeWorldOffsets.end())
        transform = glm::translate(glm::mat4(1.0f), offset->second) * transform;

    glUniformMatrix4fv(
        shader.getUniformLocation("model"),
        1,
        GL_FALSE,
        glm::value_ptr(transform)
    );

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        m_meshes[node->mMeshes[i]].draw(shader);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        drawNode(
            node->mChildren[i],
            transform,
            shader
        );
    }
}

void Model::draw(const Shader& shader) const
{
    shader.bind();

    glm::mat4 fitTransform =
        glm::scale(glm::mat4(1.0f), glm::vec3(m_modelScale)) *
        glm::translate(glm::mat4(1.0f), -m_modelCenter);

    drawNode(m_scene->mRootNode, fitTransform, shader);
}

const aiNode* Model::getNode(const std::string name) const {
    if (!m_scene || !m_scene->mRootNode)
        return nullptr;

    return m_scene->mRootNode->FindNode(name.c_str());
}
