
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstddef>
#include <stdexcept>
#include <cmath>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "bezier_curve.h"

float cameraYaw = 25.0f;
float cameraPitch = 15.0f;
float cameraDistance = 7.0f;

float lastMouseX = 0.0f;
float lastMouseY = 0.0f;

bool firstMouse = true;
bool dragging = false;

int cameraMode = 0; // go through project-files/camera-view-points.txt for better understanding

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
};

struct Mesh
{
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    GLsizei indexCount = 0;

    void create(const aiMesh* source)
    {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        vertices.reserve(source->mNumVertices);

        for (unsigned int i = 0; i < source->mNumVertices; ++i)
        {
            Vertex vertex{};

            vertex.position = glm::vec3(
                source->mVertices[i].x,
                source->mVertices[i].y,
                source->mVertices[i].z
            );

            if (source->HasNormals())
            {
                vertex.normal = glm::vec3(
                    source->mNormals[i].x,
                    source->mNormals[i].y,
                    source->mNormals[i].z
                );
            }
            else
            {
                vertex.normal = glm::vec3(0, 1, 0);
            }

            vertices.push_back(vertex);
        }

        for (unsigned int i = 0; i < source->mNumFaces; ++i)
        {
            const aiFace& face = source->mFaces[i];

            for (unsigned int j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }

        indexCount = static_cast<GLsizei>(indices.size());

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(
            GL_ARRAY_BUFFER,
            vertices.size() * sizeof(Vertex),
            vertices.data(),
            GL_STATIC_DRAW
        );

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int),
            indices.data(),
            GL_STATIC_DRAW
        );

        glVertexAttribPointer(
            0, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, position))
        );
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, normal))
        );
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw() const
    {
        glBindVertexArray(vao);
        glDrawElements(
            GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr
        );
        glBindVertexArray(0);
    }

    void destroy()
    {
        glDeleteBuffers(1, &ebo);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
};

std::string readTextFile(const std::string& path)
{
    std::ifstream file(path);

    if (!file)
        throw std::runtime_error("Cannot open file: " + path);

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

GLuint compileShader(GLenum type, const std::string& source)
{
    GLuint shader = glCreateShader(type);
    const char* text = source.c_str();

    glShaderSource(shader, 1, &text, nullptr);
    glCompileShader(shader);

    GLint success = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char log[2048]{};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compilation error:\n" << log << '\n';
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

GLuint createProgram(
    const std::string& vertexSource,
    const std::string& fragmentSource)
{
    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    if (!vertex || !fragment)
    {
        if (vertex) glDeleteShader(vertex);
        if (fragment) glDeleteShader(fragment);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    GLint success = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        char log[2048]{};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::cerr << "Program linking error:\n" << log << '\n';
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    if (!success)
    {
        glDeleteProgram(program);
        return 0;
    }

    return program;
}

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


struct Model
{
    Assimp::Importer importer;
    const aiScene* scene = nullptr;
    std::vector<Mesh> meshes;

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

        meshes.resize(scene->mNumMeshes);

        for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
        {
            meshes[i].create(scene->mMeshes[i]);
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
        GLuint program) const
    {
        glm::mat4 transform =
            parentTransform * toGlm(node->mTransformation);

        GLint modelLocation =
            glGetUniformLocation(program, "model");

        glUniformMatrix4fv(
            modelLocation,
            1,
            GL_FALSE,
            glm::value_ptr(transform)
        );

        for (unsigned int i = 0; i < node->mNumMeshes; ++i)
        {
            meshes[node->mMeshes[i]].draw();
        }

        for (unsigned int i = 0; i < node->mNumChildren; ++i)
        {
            drawNode(
                node->mChildren[i],
                transform,
                program
            );
        }
    }

    void draw(GLuint program) const
    {
        glm::mat4 fitTransform =
            glm::scale(
                glm::mat4(1.0f),
                glm::vec3(modelScale)
            ) *
            glm::translate(
                glm::mat4(1.0f),
                -modelCenter
            );

        drawNode(scene->mRootNode, fitTransform, program);
    }

    void destroy()
    {
        for (Mesh& mesh : meshes)
        {
            mesh.destroy();
        }
    }
};

void mouseButtonCallback(
    GLFWwindow* window,
    int button,
    int action,
    int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        dragging = (action == GLFW_PRESS);
        firstMouse = true;
    }
}

void cursorPositionCallback(
    GLFWwindow* window,
    double xpos,
    double ypos)
{
    if (!dragging)
        return;

    if (firstMouse)
    {
        lastMouseX = static_cast<float>(xpos);
        lastMouseY = static_cast<float>(ypos);
        firstMouse = false;
        return;
    }

    float dx = static_cast<float>(xpos) - lastMouseX;
    float dy = static_cast<float>(ypos) - lastMouseY;

    cameraYaw += dx * 0.3f;
    cameraPitch -= dy * 0.3f;

    cameraPitch = glm::clamp(cameraPitch, -85.0f, 85.0f);

    lastMouseX = static_cast<float>(xpos);
    lastMouseY = static_cast<float>(ypos);
}

void scrollCallback(
    GLFWwindow* window,
    double xoffset,
    double yoffset)
{
    cameraDistance -= static_cast<float>(yoffset) * 0.5f;
    cameraDistance = glm::clamp(cameraDistance, 2.0f, 30.0f);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        1280, 720, "Cable Car Through the Alps", nullptr, nullptr
    );

    if (!window)
    {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetScrollCallback(window, scrollCallback);

    glfwSwapInterval(1);

    if (!gladLoadGLLoader(
        reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, 1280, 720);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    try
    {
        const std::string root = PROJECT_ROOT;

        GLuint program = createProgram(
            readTextFile(root + "/shaders/basic.vert"),
            readTextFile(root + "/shaders/basic.frag")
        );

        if (!program)
            throw std::runtime_error("Could not create shader program");

        Model cableCar;

        if (!cableCar.load(root + "/assets/cable_car/scene.gltf"))
            throw std::runtime_error("Could not load cable-car model");

        GLuint curveProgram = createProgram(readTextFile(root + "/shaders/curve.vert"), readTextFile(root + "/shaders/curve.frag"));
        if (!curveProgram) {
            throw std::runtime_error("Could not create curve program");
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

        glUseProgram(program);

        glUniformMatrix4fv(
            glGetUniformLocation(program, "view"),
            1, GL_FALSE, glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(program, "projection"),
            1, GL_FALSE, glm::value_ptr(projection)
        );

        glUniform3fv(
            glGetUniformLocation(program, "viewPos"),
            1, glm::value_ptr(cameraPosition)
        );

        glUniform3f(
            glGetUniformLocation(program, "lightDir"),
            -0.5f, -1.0f, -0.3f
        );

        glUniform3f(
            glGetUniformLocation(program, "objectColor"),
            0.85f, 0.78f, 0.65f
        );
        glm::vec3 controlPoints[] = {
            glm::vec3(-3.0f, 0.0f, 0.0f),
            glm::vec3(-1.0f, 3.0f, 0.0f),
            glm::vec3( 1.0f, 3.0f, 0.0f),
            glm::vec3( 3.0f, 0.0f, 0.0f),
            glm::vec3(4.0f, 5.0f, 0.0f),
            glm::vec3(5.0f, -5.0f, 0.0f),
            glm::vec3(6.0f, 0.0f, 0.0f)
        };
        BezierCurve curves[] = {
            BezierCurve(controlPoints[0], controlPoints[1], controlPoints[2], controlPoints[3]),
            BezierCurve(controlPoints[3], controlPoints[4], controlPoints[5], controlPoints[6])
        };
        while (!glfwWindowShouldClose(window))
        {
            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
                glfwSetWindowShouldClose(window, true);

            glClearColor(0.25f, 0.45f, 0.65f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(program);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
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

            glUniformMatrix4fv(
                glGetUniformLocation(program, "view"),
                1,
                GL_FALSE,
                glm::value_ptr(view)
            );

            glUniform3fv(
                glGetUniformLocation(program, "viewPos"),
                1,
                glm::value_ptr(cameraPosition)
            );

            cableCar.draw(program);

            // start bezier curve
            glUseProgram(curveProgram);

            glUniformMatrix4fv(
                glGetUniformLocation(curveProgram, "mvp"),
                1, GL_FALSE, glm::value_ptr(projection * view)
            );

            for (const BezierCurve& curve : curves) {
                curve.draw();
            }
            // end bezier curve

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        cableCar.destroy();
        glDeleteProgram(program);
        glDeleteProgram(curveProgram);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
