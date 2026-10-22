#pragma once

#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <string>
#include <vector>

class Model {
    public:
        bool load(const std::string& path);
        void draw(const Shader& shader, GLint modelLocation);

    private:
        static glm::mat4 toGlm(const aiMatrix4x4& a);
        static Mesh convertMesh(const aiMesh& source, const aiScene& scene);
        void calculateBounds(
            const aiNode* node,
            const aiMatrix4x4& parentTransform,
            glm::vec3& minimum,
            glm::vec3& maximum,
            bool& firstVertex);

        void drawNode(
            const aiNode* node,
            const glm::mat4& parentTransform,
            const Shader& shader,
            GLint modelLocation);

        Assimp::Importer importer;
        const aiScene* scene = nullptr;
        std::vector<GPUMesh> meshes;

        glm::vec3 boundsMin{ 0.0f };
        glm::vec3 boundsMax{ 0.0f };
        glm::vec3 modelCenter{ 0.0f };
        float modelScale = 1.0f;
};
