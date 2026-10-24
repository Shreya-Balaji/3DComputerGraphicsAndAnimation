#pragma once

#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <string>
#include <vector>
#include <unordered_map>

class Model {
    public:
        Model(const std::string& path, bool autoScale = true);
        void load(const std::string& path);
        void draw(const Shader& shader) const;
        void setWorldTransform(const glm::vec3& position,float scale, const glm::vec3& rotationDegrees = glm::vec3(0.0f));
        const aiNode* getNode(const std::string name) const;
        bool setNodeWorldOffset(
            const std::string& name,
            const glm::vec3 offset
        );
    private:
        void calculateBounds(
            const aiNode* node,
            const aiMatrix4x4& parentTransform,
            glm::vec3& minimum,
            glm::vec3& maximum,
            bool& firstVertex) const;
            bool m_autoScale = true;

        void drawNode(
            const aiNode* node,
            const glm::mat4& parentTransform,
            const Shader& shader
        ) const;


        glm::vec3 m_worldPosition{ 0.0f };
        float m_worldScale = 1.0f;
        glm::vec3 m_worldRotation{ 0.0f };

        Assimp::Importer m_importer;
        const aiScene* m_scene = nullptr;
        std::vector<GPUMesh> m_meshes;
        std::unordered_map<const aiNode*, glm::vec3> m_nodeWorldOffsets;

        glm::vec3 m_boundsMin{ 0.0f };
        glm::vec3 m_boundsMax{ 0.0f };
        glm::vec3 m_modelCenter{ 0.0f };
        float m_modelScale = 1.0f;
};
