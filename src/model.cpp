
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

const glm::vec3 DEFAULT_COLOR = glm::vec3(0.85f, 0.78f, 0.65f);

glm::mat4 Model::toGlm(const aiMatrix4x4& a)
{
    glm::mat4 m(1.0f);

    m[0][0] = a.a1; m[1][0] = a.a2;
    m[2][0] = a.a3; m[3][0] = a.a4;

    m[0][1] = a.b1; m[1][1] = a.b2;
    m[2][1] = a.b3; m[3][1] = a.b4;

    m[0][2] = a.c1; m[1][2] = a.c2;
    m[2][2] = a.c3; m[3][2] = a.c4;

    m[0][3] = a.d1; m[1][3] = a.d2;
    m[2][3] = a.d3; m[3][3] = a.d4;

    return m;
}

Mesh Model::convertMesh(const aiMesh& source, const aiScene& scene)
{
    Mesh result;

    result.vertices.reserve(source.mNumVertices);

    for (unsigned int i = 0; i < source.mNumVertices; ++i) {
        Vertex vertex{};

        vertex.position = {
            source.mVertices[i].x,
            source.mVertices[i].y,
            source.mVertices[i].z
        };

        vertex.normal = source.HasNormals()
            ? glm::vec3(source.mNormals[i].x,
                        source.mNormals[i].y,
                        source.mNormals[i].z)
            : glm::vec3(0.0f, 1.0f, 0.0f);

        vertex.texCoord = source.HasTextureCoords(0)
            ? glm::vec2(source.mTextureCoords[0][i].x,
                        source.mTextureCoords[0][i].y)
            : glm::vec2(0.0f);

        result.vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < source.mNumFaces; ++i) {
        const aiFace& face = source.mFaces[i];

        if (face.mNumIndices != 3)
            throw std::runtime_error("Expected triangle geometry");

        result.triangles.push_back(glm::uvec3(
            face.mIndices[0],
            face.mIndices[1],
            face.mIndices[2]
        ));
    }

    if (source.mMaterialIndex < scene.mNumMaterials) {
        const aiMaterial* material = scene.mMaterials[source.mMaterialIndex];
        aiColor3D color;

        if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
            result.material.kd = glm::vec3(color.r, color.g, color.b);
        }
        else {
            result.material.kd = DEFAULT_COLOR;
        }

        if (material->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
            result.material.ks = glm::vec3(color.r, color.g, color.b);
        }
        if (glm::length(result.material.ks) < 0.001f) {
            result.material.ks = glm::vec3(0.25f);
        }

        float shininess = 0.0f;
        if (material->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS && std::isfinite(shininess) && shininess > 0.0f) {
            result.material.shininess = shininess;
        }
        else {
            result.material.shininess = 32.0f;
        }
    }
    else {
        result.material.kd = DEFAULT_COLOR;
    }

    return result;
}

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
        Mesh cpuMesh = convertMesh(*scene->mMeshes[i], *scene);

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
        parentTransform * toGlm(node->mTransformation);

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
