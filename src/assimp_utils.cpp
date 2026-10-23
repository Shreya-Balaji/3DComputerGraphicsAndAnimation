#include "assimp_utils.h"

#include <assimp/scene.h>
#include <assimp/material.h>
#include <stdexcept>
#include <iostream>
namespace {
const glm::vec3 DEFAULT_COLOR{0.85f, 0.78f, 0.65f};
}

glm::mat4 toGLM(const aiMatrix4x4& a)
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

Mesh toCPUMesh(const aiMesh& source, const aiScene& scene)
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

void printNodeHierarchy(const aiNode* node, int depth)
{
    std::string indent(depth * 2, ' ');

    std::cout << indent << "Node: "
              << node->mName.C_Str()
              << " (meshes: " << node->mNumMeshes << ")\n";

    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        std::cout << indent << "  Mesh index: "
                  << node->mMeshes[i] << "\n";
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        printNodeHierarchy(node->mChildren[i], depth + 1);
    }
}
