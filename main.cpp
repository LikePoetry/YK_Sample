#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// 顶点着色器
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// 片段着色器
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.3, 0.7, 0.3, 1.0); // 绿色
}
)";

// 相机类
class Camera {
public:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    float yaw;
    float pitch;
    float speed;
    float sensitivity;

    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
        : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch), speed(2.5f), sensitivity(0.1f) {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix() {
        return glm::lookAt(position, position + front, up);
    }

    void processMouseMovement(float xOffset, float yOffset) {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 newFront;
        newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        newFront.y = sin(glm::radians(pitch));
        newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        front = glm::normalize(newFront);

        right = glm::normalize(glm::cross(front, worldUp));
        up = glm::normalize(glm::cross(right, front));
    }
};

// 全局变量
Camera camera(glm::vec3(0.0f, 50.0f, 200.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -30.0f);
bool firstMouse = true;
float lastX = 400, lastY = 300;
bool rightMousePressed = false;

// 鼠标回调函数
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!rightMousePressed) return;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos; // 反转 y 坐标

    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xOffset, yOffset);
}

// 鼠标按键回调函数
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMousePressed = true;
            firstMouse = true; // 重置鼠标初始位置
        } else if (action == GLFW_RELEASE) {
            rightMousePressed = false;
        }
    }
}

// 鼠标滚轮回调函数
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    camera.position += static_cast<float>(yOffset) * camera.front * 0.5f;
}

// 定义补丁类
class TerrainPatch {
public:
    unsigned int VAO, VBO, EBO;
    std::vector<float> vertices;
    std::vector<std::vector<unsigned int>> lodIndices; // 每个 LOD 的索引
    glm::vec3 center; // 补丁中心位置

    TerrainPatch(int gridSize, float spacing, glm::vec3 patchCenter) : center(patchCenter) {
        generateVertices(gridSize, spacing);
        generateLODIndices(gridSize);
        setupBuffers();
    }

    ~TerrainPatch() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    void render(int lodLevel) {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, lodIndices[lodLevel].size() * sizeof(unsigned int), lodIndices[lodLevel].data(), GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, lodIndices[lodLevel].size(), GL_UNSIGNED_INT, 0);
    }

private:
    void generateVertices(int gridSize, float spacing) {
        for (int z = 0; z <= gridSize; ++z) {
            for (int x = 0; x <= gridSize; ++x) {
                vertices.push_back(center.x + x * spacing); // x 坐标
                vertices.push_back(0.0f);                  // y 坐标（高度）
                vertices.push_back(center.z + z * spacing); // z 坐标
            }
        }
    }

    void generateLODIndices(int gridSize) {
        int maxLOD = log2(gridSize); // 最大 LOD 层级
        for (int lod = 0; lod <= maxLOD; ++lod) {
            int step = 1 << lod; // LOD 步长
            std::vector<unsigned int> indices;
            for (int z = 0; z < gridSize; z += step) {
                for (int x = 0; x < gridSize; x += step) {
                    int topLeft = z * (gridSize + 1) + x;
                    int topRight = topLeft + step;
                    int bottomLeft = (z + step) * (gridSize + 1) + x;
                    int bottomRight = bottomLeft + step;

                    // 添加两个三角形
                    indices.push_back(topLeft);
                    indices.push_back(bottomLeft);
                    indices.push_back(topRight);

                    indices.push_back(topRight);
                    indices.push_back(bottomLeft);
                    indices.push_back(bottomRight);
                }
            }
            lodIndices.push_back(indices);
        }
    }

    void setupBuffers() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
};

// 计算 LOD 层级
int calculateLOD(const glm::vec3& cameraPosition, const glm::vec3& patchCenter) {
    float distance = glm::length(cameraPosition - patchCenter);

    if (distance < 50.0f) return 0; // 高细节
    if (distance < 200.0f) return 1; // 中等细节
    return 2; // 低细节
}

int main() {
    // 初始化 GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Terrain Rendering with CLOD", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // 设置鼠标回调
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // 创建着色器
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 创建多个地形补丁
    std::vector<TerrainPatch> terrainPatches;
    int patchGridSize = 64; // 每个补丁的网格大小
    float patchSpacing = 64.0f; // 补丁间距
    for (int z = 0; z < 10; ++z) {
        for (int x = 0; x < 10; ++x) {
            glm::vec3 patchCenter(x * patchSpacing, 0.0f, z * patchSpacing);
            terrainPatches.emplace_back(patchGridSize, 1.0f, patchCenter);
        }
    }

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);

    // 渲染循环
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // 渲染每个地形补丁
        for (auto& patch : terrainPatches) {
            int lodLevel = calculateLOD(camera.position, patch.center);
            patch.render(lodLevel);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}