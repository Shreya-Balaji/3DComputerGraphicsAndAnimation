#include "mesh.h"
#include <framework/disable_all_warnings.h>
DISABLE_WARNINGS_PUSH()
#include <fmt/format.h>
DISABLE_WARNINGS_POP()
#include <cstddef>
#include <utility>
#include <vector>
#include <stdexcept>

GPUMaterial::GPUMaterial(const Material& material) :
    kd(material.kd),
    ks(material.ks),
    shininess(material.shininess),
    transparency(material.transparency)
{}

GPUMesh::GPUMesh(const Mesh& cpuMesh)
{
    // Create uniform buffer to store mesh material (https://learnopengl.com/Advanced-OpenGL/Advanced-GLSL)
    GPUMaterial gpuMaterial(cpuMesh.material);
    glGenBuffers(1, &m_uboMaterial);
    glBindBuffer(GL_UNIFORM_BUFFER, m_uboMaterial);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(GPUMaterial), &gpuMaterial, GL_STATIC_READ);

    // Figure out if this mesh has texture coordinates

    m_hasTextureCoords = false;

    if (cpuMesh.material.kdTexture) {
        Image& image = *cpuMesh.material.kdTexture;

        GLenum format = GL_RGB;
        GLenum internalFormat = GL_RGB8;

        switch (image.channels) {
        case 1:
            format = GL_RED;
            internalFormat = GL_R8;
            break;
        case 3:
            format = GL_RGB;
            internalFormat = GL_RGB8;
            break;
        case 4:
            format = GL_RGBA;
            internalFormat = GL_RGBA8;
            break;
        default:
            throw std::runtime_error(
                "Unsupported texture channel count");
        }

        glGenTextures(1, &m_texture);
        glBindTexture(GL_TEXTURE_2D, m_texture);

        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
            GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(
            GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        GLint previousAlignment = 4;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &previousAlignment);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            static_cast<GLint>(internalFormat),
            image.width,
            image.height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            image.get_data());

        glGenerateMipmap(GL_TEXTURE_2D);

        glPixelStorei(GL_UNPACK_ALIGNMENT, previousAlignment);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_hasTextureCoords = true;
    }


    // Create VAO and bind it so subsequent creations of VBO and IBO are bound to this VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    // Create vertex buffer object (VBO)
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(cpuMesh.vertices.size() * sizeof(decltype(cpuMesh.vertices)::value_type)), cpuMesh.vertices.data(), GL_STATIC_DRAW);

    // Create index buffer object (IBO)
    glGenBuffers(1, &m_ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(cpuMesh.triangles.size() * sizeof(decltype(cpuMesh.triangles)::value_type)), cpuMesh.triangles.data(), GL_STATIC_DRAW);

    // Tell OpenGL that we will be using vertex attributes 0, 1 and 2.
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    // We tell OpenGL what each vertex looks like and how they are mapped to the shader (location = ...).
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    // Reuse all attributes for each instance
    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 0);
    glVertexAttribDivisor(2, 0);

    // Each triangle has 3 vertices.
    m_numIndices = static_cast<GLsizei>(3 * cpuMesh.triangles.size());
 
}

GPUMesh::GPUMesh(GPUMesh&& other)
{
    moveInto(std::move(other));
}

GPUMesh::~GPUMesh()
{
    freeGpuMemory();
}

GPUMesh& GPUMesh::operator=(GPUMesh&& other)
{
    moveInto(std::move(other));
    return *this;
}

std::vector<GPUMesh> GPUMesh::loadMeshGPU(std::filesystem::path filePath, bool normalize) {
    if (!std::filesystem::exists(filePath))
        throw MeshLoadingException(fmt::format("File {} does not exist", filePath.string().c_str()));

    // Generate GPU-side meshes for all sub-meshes
    std::vector<Mesh> subMeshes = loadMesh(filePath, { .normalizeVertexPositions = normalize });
    std::vector<GPUMesh> gpuMeshes;
    for (const Mesh& mesh : subMeshes) { gpuMeshes.emplace_back(mesh); }

    return gpuMeshes;
}

bool GPUMesh::hasTextureCoords() const
{
    return m_hasTextureCoords;
}


void GPUMesh::draw(const Shader& drawingShader) const
{
    drawingShader.bindUniformBlock("Material", 0, m_uboMaterial);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    const GLint textureLocation =
        drawingShader.getUniformLocation("kdTexture");
    if (textureLocation >= 0) {
        glUniform1i(textureLocation, 0);
    }

    const GLint hasTextureLocation =
        drawingShader.getUniformLocation("hasKdTexture");
    if (hasTextureLocation >= 0) {
        glUniform1i(
            hasTextureLocation,
            m_texture != INVALID ? 1 : 0);
    }

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_numIndices,
        GL_UNSIGNED_INT, nullptr);

    glBindTexture(GL_TEXTURE_2D, 0);
}


void GPUMesh::moveInto(GPUMesh&& other)
{
    freeGpuMemory();
    m_numIndices = other.m_numIndices;
    m_hasTextureCoords = other.m_hasTextureCoords;
    m_ibo = other.m_ibo;
    m_vbo = other.m_vbo;
    m_vao = other.m_vao;
    m_uboMaterial = other.m_uboMaterial;
    m_texture = other.m_texture;
    other.m_numIndices = 0;
    other.m_hasTextureCoords =false;
    other.m_ibo = INVALID;
    other.m_vbo = INVALID;
    other.m_vao = INVALID;
    other.m_uboMaterial = INVALID;
    other.m_texture = INVALID;
}

void GPUMesh::freeGpuMemory()
{
    if (m_vao != INVALID)
        glDeleteVertexArrays(1, &m_vao);
    if (m_vbo != INVALID)
        glDeleteBuffers(1, &m_vbo);
    if (m_ibo != INVALID)
        glDeleteBuffers(1, &m_ibo);
    if (m_uboMaterial != INVALID)
        glDeleteBuffers(1, &m_uboMaterial);
    if (m_texture != INVALID)
        glDeleteTextures(1, &m_texture);
}
