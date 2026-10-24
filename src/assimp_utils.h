#pragma once

#include <framework/mesh.h>
#include <assimp/matrix4x4.h>
#include <glm/mat4x4.hpp>
#include <filesystem>

struct aiMesh;
struct aiScene;

// Convert triangulated Assimp geometry and material values to a CPU mesh.
Mesh toCPUMesh(const aiMesh& source, const aiScene& scene, const std::filesystem::path& modelDirectory);
glm::mat4 toGLM(const aiMatrix4x4& matrix);
