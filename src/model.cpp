
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <cmath>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include "model.h"
#include "assimp_utils.h"

void Model::calculateBounds(
    const aiNode* node,
    const aiMatrix4x4& parentTransform,
    glm::vec3& minimum,
    glm::vec3& maximum,
    bool& firstVertex)
{
    aiMatrix4x4 transform =
        parentTransform * node->mTransformation;

    for (unsigned int i = 0; i < node->mNumMeshes; ++i)
    {
        const aiMesh* mesh =
            scene->mMeshes[node->mMeshes[i]];

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

bool Model::load(const std::string& path)
{
    scene = importer.ReadFile(
        path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality
    );

    if (!scene || !scene->mRootNode)
    {
        std::cerr << "Assimp error: "
            << importer.GetErrorString() << '\n';
        return false;
    }

    meshes.clear();
    meshes.reserve(scene->mNumMeshes);

    for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
        Mesh cpuMesh = toCPUMesh(*scene->mMeshes[i], *scene);

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

        meshes.emplace_back(cpuMesh);
    }

    // Calculate the complete model bounds, including node transforms.
    bool firstVertex = true;
    aiMatrix4x4 identity;

    calculateBounds(
        scene->mRootNode,
        identity,
        boundsMin,
        boundsMax,
        firstVertex
    );

    if (firstVertex)
    {
        std::cerr << "Model contains no vertices.\n";
        return false;
    }

    modelCenter = (boundsMin + boundsMax) * 0.5f;

    glm::vec3 dimensions = boundsMax - boundsMin;

    float maxDimension = glm::max(
        dimensions.x,
        glm::max(dimensions.y, dimensions.z)
    );

    if (maxDimension <= 0.000001f)
    {
        std::cerr << "Invalid model dimensions.\n";
        return false;
    }

    // Fit the largest dimension to approximately 4 world units.
    modelScale = 4.0f / maxDimension;

    std::cout << "Model loaded successfully!\n";
    std::cout << "Meshes: " << scene->mNumMeshes << '\n';
    std::cout << "Materials: " << scene->mNumMaterials << '\n';

    std::cout << "Model dimensions: "
        << dimensions.x << ", "
        << dimensions.y << ", "
        << dimensions.z << '\n';

    std::cout << "Model center: "
        << modelCenter.x << ", "
        << modelCenter.y << ", "
        << modelCenter.z << '\n';

    std::cout << "Automatic scale: "
        << modelScale << '\n';

    return true;
}
void Model::drawNode(
    const aiNode* node,
    const glm::mat4& parentTransform,
    const Shader& shader,
    GLint modelLocation)
{
    glm::mat4 transform =
        parentTransform * toGLM(node->mTransformation);

    glUniformMatrix4fv(
        modelLocation,
        1,
        GL_FALSE,
        glm::value_ptr(transform)
    );

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        meshes[node->mMeshes[i]].draw(shader);
    }

    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        drawNode(
            node->mChildren[i],
            transform,
            shader,
            modelLocation
        );
    }
}
void Model::draw(const Shader& shader, GLint modelLocation)
{
    shader.bind();

    glm::mat4 fitTransform =
        glm::scale(glm::mat4(1.0f), glm::vec3(modelScale)) *
        glm::translate(glm::mat4(1.0f), -modelCenter);

    drawNode(scene->mRootNode, fitTransform, shader, modelLocation);
}
