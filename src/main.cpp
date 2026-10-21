
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <framework/window.h>
#include <imgui/imgui.h>
#include <framework/shader.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_int2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <ostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cmath>

#include "mesh.h"
#include "bezier_curve.h"

glm::vec3 CABLE_CAR_COLOR = glm::vec3(0.85f, 0.78f, 0.65f);

float cameraYaw = 25.0f;
float cameraPitch = 15.0f;
float cameraDistance = 7.0f;

float lastMouseX = 0.0f;
float lastMouseY = 0.0f;

bool firstMouse = true;
bool dragging = false;

int cameraMode = 0; // go through project-files/camera-view-points.txt for better understanding

glm::vec3 controlPoints[] = {
    glm::vec3(-3.0f, 0.0f, 0.0f),
    glm::vec3(-1.0f, 3.0f, 0.0f),
    glm::vec3( 1.0f, 3.0f, 0.0f),
    glm::vec3( 3.0f, 0.0f, 0.0f),
    glm::vec3(4.0f, 5.0f, 0.0f),
    glm::vec3(5.0f, -5.0f, 0.0f),
    glm::vec3(6.0f, 0.0f, 0.0f)
};

glm::mat4 toGlm(const aiMatrix4x4& a)
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

Mesh convertMesh(const aiMesh& source)
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

    result.material.kd = CABLE_CAR_COLOR;

    return result;
}

struct Model
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
    std::vector<GPUMesh> meshes;

    glm::vec3 boundsMin{ 0.0f };
    glm::vec3 boundsMax{ 0.0f };
    glm::vec3 modelCenter{ 0.0f };
    float modelScale = 1.0f;

    void calculateBounds(
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

    bool load(const std::string& path)
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
            Mesh cpuMesh = convertMesh(*scene->mMeshes[i]);
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

    void drawNode(
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

    void draw(const Shader& shader, GLint modelLocation)
    {
        shader.bind();

        glm::mat4 fitTransform =
            glm::scale(glm::mat4(1.0f), glm::vec3(modelScale)) *
            glm::translate(glm::mat4(1.0f), -modelCenter);

        drawNode(scene->mRootNode, fitTransform, shader, modelLocation);
    }
};

void mouseButtonCallback(int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        dragging = (action == GLFW_PRESS);
        firstMouse = true;
    }
}

void mouseMoveCallback(const glm::vec2& position)
{
    if (!dragging)
        return;

    if (firstMouse)
    {
        lastMouseX = position.x;
        lastMouseY = position.y;
        firstMouse = false;
        return;
    }
    if (!dragging)
        return;

    if (firstMouse) {
        lastMouseX = position.x;
        lastMouseY = position.y;
        firstMouse = false;
        return;
    }

    float dx = position.x - lastMouseX;
    float dy = position.y - lastMouseY;

    cameraYaw += dx * 0.3f;
    cameraPitch += dy * 0.3f;

    cameraPitch = glm::clamp(cameraPitch, -85.0f, 85.0f);

    lastMouseX = position.x;
    lastMouseY = position.y;
}

void scrollCallback(const glm::vec2& offset)
{
    cameraDistance -= offset.y * 0.5f;
    cameraDistance = glm::clamp(cameraDistance, 2.0f, 30.0f);
}

int main()
{
    Window window(
        "Cable Car Through the Alps",
        glm::ivec2(1280, 720),
        OpenGLVersion::GL41
    );

    window.registerMouseButtonCallback(mouseButtonCallback);
    window.registerMouseMoveCallback(mouseMoveCallback);
    window.registerScrollCallback(scrollCallback);

    glEnable(GL_DEPTH_TEST);

    try
    {
        const std::string root = PROJECT_ROOT;

        Model cableCar;

        if (!cableCar.load(root + "/assets/cable_car/scene.gltf"))
            throw std::runtime_error("Could not load cable-car model");

        Shader basicShader, curveShader;
        try {
            ShaderBuilder basicBuilder;
            basicBuilder.addStage(GL_VERTEX_SHADER, root + "/shaders/basic.vert");
            basicBuilder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/basic.frag");
            basicShader = basicBuilder.build();

            ShaderBuilder curveBuilder;
            curveBuilder.addStage(GL_VERTEX_SHADER, root + "/shaders/curve.vert");
            curveBuilder.addStage(GL_FRAGMENT_SHADER, root + "/shaders/curve.frag");
            curveShader = curveBuilder.build();
        } catch (ShaderLoadingException e) {
            std::cerr << e.what() << std::endl;
        }

        glm::vec3 cameraPosition(0.0f, 1.5f, 7.0f);

        glm::mat4 view = glm::lookAt(
            cameraPosition,
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            1280.0f / 720.0f,
            0.1f,
            1000.0f
        );

        basicShader.bind();

        glUniformMatrix4fv(
            basicShader.getUniformLocation("projection"),
            1, GL_FALSE, glm::value_ptr(projection)
        );

        glUniform3f(
           basicShader.getUniformLocation("lightDir"),
            -0.5f, -1.0f, -0.3f
        );

        BezierCurve curves[] = {
            BezierCurve(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]),
            BezierCurve(controlPoints[3], controlPoints[4], controlPoints[5], controlPoints[6])
        };

        while (!window.shouldClose())
        {
            window.updateInput();
            if (window.isKeyPressed(GLFW_KEY_ESCAPE))
                window.close();

            if (!window.isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT) || ImGui::GetIO().WantCaptureMouse) {
                dragging = false;
                firstMouse = true;
            }

            auto size = window.getFrameBufferSize();
            glViewport(0, 0, size.x, size.y);

            glClearColor(0.25f, 0.45f, 0.65f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ImGui::Begin("Camera View Points Controls");

            const char* cameraModes[] = {
                "Orbit View",
                "Top View",
                "Follow View"
            };

            ImGui::Combo(
                "Camera Mode",
                &cameraMode,
                cameraModes,
                IM_ARRAYSIZE(cameraModes)
            );

            ImGui::Separator();
            ImGui::Text("Orbit Camera Settings");

            ImGui::SliderFloat(
                "Yaw",
                &cameraYaw,
                -180.0f,
                180.0f
            );

            ImGui::SliderFloat(
                "Pitch",
                &cameraPitch,
                -85.0f,
                85.0f
            );

            ImGui::SliderFloat(
                "Distance",
                &cameraDistance,
                2.0f,
                30.0f
            );

            ImGui::End();

            glm::vec3 cameraPosition;
            glm::vec3 cameraTarget(0.0f);

            switch (cameraMode)
            {
                case 0:
                {
                    float yaw = glm::radians(cameraYaw);
                    float pitch = glm::radians(cameraPitch);

                    cameraPosition = glm::vec3(
                        cameraDistance * cos(pitch) * sin(yaw),
                        cameraDistance * sin(pitch),
                        cameraDistance * cos(pitch) * cos(yaw)
                    );
                    break;
                }

                case 1:
                {
                    cameraPosition = glm::vec3(0.0f, 12.0f, 0.01f);
                    break;
                }

                case 2:
                {
                    cameraPosition = glm::vec3(0.0f, 2.5f, 8.0f);
                    break;
                }

                default:
                {
                    cameraMode = 0;
                    cameraPosition = glm::vec3(0.0f, 0.0f, cameraDistance);
                    break;
                }
            }

            glm::mat4 view = glm::lookAt(
                cameraPosition,
                cameraTarget,
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            // START draw cable car
            basicShader.bind();
            glUniformMatrix4fv(
                basicShader.getUniformLocation("view"),
                1,
                GL_FALSE,
                glm::value_ptr(view)
            );

            glUniform3fv(
                basicShader.getUniformLocation("viewPos"),
                1,
                glm::value_ptr(cameraPosition)
            );

            cableCar.draw(basicShader, basicShader.getUniformLocation("model"));
            // END draw cable car

            // START draw Bezier curve
            curveShader.bind();
            glUniformMatrix4fv(
                curveShader.getUniformLocation("mvp"),
                1, GL_FALSE, glm::value_ptr(projection * view)
            );

            for (const BezierCurve& curve : curves) {
                curve.draw();
            }
            // END draw bezier curve

            window.swapBuffers();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }
    return 0;
}
