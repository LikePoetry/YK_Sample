#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

class Mesh
{
public:
    Mesh(std::vector<float> vertices, std::vector<unsigned int> indices)
    {
        this->vertices = vertices;
        this->indices = indices;
        indices_count= indices.size();
        // 生成 VAO、VBO 和 EBO

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float)*indices.size(), indices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }

    unsigned int getVAO() const { return VAO; }
    unsigned int getEBO() const { return EBO; }
    int getIndicesCount() const { return indices_count; }

private:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int indices_count;
    unsigned int VAO, VBO, EBO;
};

// 顶点着色器
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

// 片段着色器
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // 白色
}
)";

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Triangle with Modern OpenGL", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 模块尺寸，
    float half_size = 0.5f;

    std::vector<float> b_vertices(27); // 9*3=27
    float offsetX = 0.0f;
    float offsetY = 0.0f;

    b_vertices[0] = offsetX;
    b_vertices[1] = offsetY;
    b_vertices[2] = 0.0f;
    b_vertices[3] = offsetX - half_size;
    b_vertices[4] = offsetY - half_size;
    b_vertices[5] = 0.0f;
    b_vertices[6] = offsetX - half_size;
    b_vertices[7] = offsetY;
    b_vertices[8] = 0.0f;
    b_vertices[9] = offsetX - half_size;
    b_vertices[10] = offsetY + half_size;
    b_vertices[11] = 0.0f;
    b_vertices[12] = offsetX;
    b_vertices[13] = offsetY + half_size;
    b_vertices[14] = 0.0f;
    b_vertices[15] = offsetX + half_size;
    b_vertices[16] = offsetY + half_size;
    b_vertices[17] = 0.0f;
    b_vertices[18] = offsetX + half_size;
    b_vertices[19] = offsetY;
    b_vertices[20] = 0.0f;
    b_vertices[21] = offsetX + half_size;
    b_vertices[22] = offsetY - half_size;
    b_vertices[23] = 0.0f;
    b_vertices[24] = offsetX;
    b_vertices[25] = offsetY - half_size;
    b_vertices[26] = 0.0f;

    std::vector<unsigned int> b_indices(10); // 9个索引
    b_indices[0] = 0;
    b_indices[1] = 1;
    b_indices[2] = 2;
    b_indices[3] = 3;
    b_indices[4] = 4;
    b_indices[5] = 5;
    b_indices[6] = 6;
    b_indices[7] = 7;
    b_indices[8] = 8;
    b_indices[9] = 1; // 关闭环

    Mesh mesh(b_vertices, b_indices);

    // 创建和编译着色器
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

    // 设置线框模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(mesh.getVAO()); // 绑定 VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.getEBO());                              // 绑定第一个索引缓冲区
        glDrawElements(GL_TRIANGLE_FAN, mesh.getIndicesCount(), GL_UNSIGNED_INT, 0); // 绘制三角形

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // 解绑索引缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}