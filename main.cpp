#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "terrain.h"
#include "geomipmapping.h"

#include "tiffio.h"

class HeightMap
{
public:
    HeightMap(const char *filename)
    {
        TIFF *tif = TIFFOpen(filename, "r");
        if (tif == NULL)
        {
            std::cerr << "Error opening TIFF file: " << filename << std::endl;
            return;
        }
        uint16_t bitsPerSample;
        TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &Width);
        TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &Height);
        TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);

        HeightData.resize(Width * Height);
        if (bitsPerSample == 32)
        {
            float *buf = (float *)_TIFFmalloc(Width * sizeof(float));
            for (uint32_t row = 0; row < Height; row++)
            {
                TIFFReadScanline(tif, buf, row);
                // 示例：打印第一行前5个像素
                for (uint32_t col = 0; col < Width; col++)
                {
                    // 处理每个像素的浮点值
                    HeightData[row * Width + col] = buf[col];
                }
            }
            _TIFFfree(buf);
        }
        else
        {
            printf("不支持位深: %u\n", bitsPerSample);
        }

        TIFFClose(tif);
    }

    ~HeightMap()
    {
        HeightData.clear();
    }

    float getHeight(int x, int y)
    {
        if (x < 0 || x >= Width || y < 0 || y >= Height)
        {
            return 0.0f; // 返回默认高度
        }
        return HeightData[y * Width + x];
    }

private:
    uint32_t Width;
    uint32_t Height;
    std::vector<float> HeightData;
};

struct LandPatch
{
    unsigned int VAO; // 顶点数组对象
    float *vertices;  // 补丁顶点信息;
    int iLOD;         // 当前补丁应该使用的等级，与相机距离有关
    float fDistance;  // 距离相机的距离
    float ix;
    float iy;
    float imin_x;
    float imin_y;
    float imax_x;
    float imax_y;
};

struct LandPatchIndex
{
    unsigned int *indices; // 衍生的地形补丁索引
    int indices_count;     // 衍生的地形补丁索引数量
    int iLOD;              // 当前补丁应该使用的等级，与相机距离有关
};

class LandScapeMap
{
private:
    LandPatch *LandPatches;           // 衍生的地形补丁
    LandPatchIndex *LandPatchIndices; // 衍生的地形补丁索引
    int iPatchSize;                   // 衍生的地形大小
    int iNumPatchesPerSide;           // 每边的补丁数量
    int iMaxLOD;                      // 细节等级

public:
    void init()
    {

        HeightMap heightMap("heightmap.tif");
        // 读取高度图数据

        int iLOD = 0;
        int iDivisor = iPatchSize - 1;
        while (iDivisor > 2)
        {
            iDivisor = iDivisor >> 1;
            iLOD++;
        }
        iMaxLOD = iLOD;
        int half = iPatchSize / 2;
        // 计算顶点数量
        for (int32_t y = 0; y < iNumPatchesPerSide; y++)
        {
            for (int32_t x = 0; x < iNumPatchesPerSide; x++)
            {
                LandPatches[y * iNumPatchesPerSide + x].iLOD = iMaxLOD;
                LandPatches[y * iNumPatchesPerSide + x].fDistance = 0.0f;
                LandPatches[y * iNumPatchesPerSide + x].vertices = new float[iPatchSize * iPatchSize * 3];
                float i_minx = std::numeric_limits<float>::max();
                float i_miny = std::numeric_limits<float>::max();
                float i_maxx = std::numeric_limits<float>::min();
                float i_maxy = std::numeric_limits<float>::min();
                float dx = 0;
                float dy = 0;
                // 计算补丁的顶点坐标
                for (int32_t j = 0; j < iPatchSize; j++)
                {
                    for (int32_t i = 0; i < iPatchSize; i++)
                    {
                        dx = x * (iPatchSize - 1) + i;
                        dy = y * (iPatchSize - 1) + j;
                        // 坐标 x 为 y*iNumPatchesPerSide*iPatchSize
                        LandPatches[y * iNumPatchesPerSide + x].vertices[j * iPatchSize * 3 + i * 3] = dx;
                        LandPatches[y * iNumPatchesPerSide + x].vertices[j * iPatchSize * 3 + i * 3 + 1] = dy;
                        LandPatches[y * iNumPatchesPerSide + x].vertices[j * iPatchSize * 3 + i * 3 + 2] = 4000 * heightMap.getHeight(dx, dy); // heightMap.getHeight(dx, dy);
                        // std::cout << "vertices x:" << x * (iPatchSize - 1) + i << ", y:" << y * (iPatchSize - 1) + j << std::endl;
                        if (dx < i_minx)
                        {
                            i_minx = dx;
                        }
                        if (dy < i_miny)
                        {
                            i_miny = dy;
                        }
                        if (dx > i_maxx)
                        {
                            i_maxx = dx;
                        }
                        if (dy > i_maxy)
                        {
                            i_maxy = dy;
                        }
                    }
                }
                LandPatches[y * iNumPatchesPerSide + x].ix = LandPatches[y * iNumPatchesPerSide + x].vertices[(half * iPatchSize + half) * 3];
                LandPatches[y * iNumPatchesPerSide + x].iy = LandPatches[y * iNumPatchesPerSide + x].vertices[(half * iPatchSize + half) * 3 + 1];
                LandPatches[y * iNumPatchesPerSide + x].imin_x = i_minx;
                LandPatches[y * iNumPatchesPerSide + x].imin_y = i_miny;
                LandPatches[y * iNumPatchesPerSide + x].imax_x = i_maxx;
                LandPatches[y * iNumPatchesPerSide + x].imax_y = i_maxy;

                unsigned int VAO, VBO;
                // 生成 VAO、VBO
                glGenVertexArrays(1, &VAO);
                glGenBuffers(1, &VBO);

                glBindVertexArray(VAO);
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * iPatchSize * iPatchSize * 3, LandPatches[y * iNumPatchesPerSide + x].vertices, GL_STATIC_DRAW);

                glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
                glEnableVertexAttribArray(0);

                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindVertexArray(0);
                LandPatches[y * iNumPatchesPerSide + x].VAO = VAO; // 保存 VAO
            }
        }

        LandPatchIndices = new LandPatchIndex[iMaxLOD];
        for (int32_t c_lod = 0; c_lod <= iMaxLOD; c_lod++)
        {
            int ebo_per_patch = (iPatchSize - 1);
            int step = 1;
            int lod = c_lod;
            while (lod > -1)
            {
                lod--;
                ebo_per_patch = ebo_per_patch >> 1;
                step = step << 1;
            }
            step = step >> 1;

            LandPatchIndices[c_lod].indices_count = ebo_per_patch * ebo_per_patch;
            LandPatchIndices[c_lod].indices = new unsigned int[ebo_per_patch * ebo_per_patch];
            LandPatchIndices[c_lod].iLOD = c_lod;

            unsigned int *indices = new unsigned int[10];
            // 构建顶点索引;
            for (int32_t j = 0; j < ebo_per_patch; j++)
            {
                for (int32_t i = 0; i < ebo_per_patch; i++)
                {
                    indices[0] = (j * (step * 2) + 1 * step) * iPatchSize + (i * (step * 2) + 1 * step);
                    indices[1] = (j * (step * 2) + 2 * step) * iPatchSize + i * (step * 2);
                    indices[2] = (j * (step * 2) + 1 * step) * iPatchSize + i * (step * 2);
                    indices[3] = (j * (step * 2)) * iPatchSize + i * (step * 2);
                    indices[4] = ((j * (step * 2)) * iPatchSize + (i * (step * 2) + 1 * step));
                    indices[5] = ((j * (step * 2)) * iPatchSize + (i * (step * 2) + 2 * step));
                    indices[6] = ((j * (step * 2) + 1 * step) * iPatchSize + (i * (step * 2) + 2 * step));
                    indices[7] = ((j * (step * 2) + 2 * step) * iPatchSize + (i * (step * 2) + 2 * step));
                    indices[8] = ((j * (step * 2) + 2 * step) * iPatchSize + (i * (step * 2) + 1 * step));
                    indices[9] = ((j * (step * 2) + 2 * step) * iPatchSize + i * (step * 2));

                    // std::cout << "indices " << indices[0] << ", " << indices[1] << ", " << indices[2] << ", " << indices[3] << ", " << indices[4] << ", " << indices[5] << ", " << indices[6] << ", " << indices[7] << ", " << indices[8] << ", " << indices[9] << std::endl;
                    //  生成索引缓冲区
                    unsigned int EBO;
                    glGenBuffers(1, &EBO);
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * 10, indices, GL_STATIC_DRAW);
                    LandPatchIndices[c_lod].indices[j * ebo_per_patch + i] = EBO; // 保存索引缓冲区
                }
            }
            delete[] indices; // 释放索引缓冲区
        }
    }

    void render(glm::vec3 eye_position, glm::vec3 target = glm::vec3(0, 0, 0), float resolution = 0)
    {
        // float fDistance = 0.0f;                                  // 平均距离，
        // float min_distance = std::numeric_limits<double>::max(); // 距离最小值
        // float max_distance = std::numeric_limits<double>::min(); // 距离最大值
        // for (int32_t y = 0; y < iNumPatchesPerSide; y++)
        // {
        //     for (int32_t x = 0; x < iNumPatchesPerSide; x++)
        //     {
        //         float d = glm::distance(eye_position, glm::vec3(LandPatches[y * iNumPatchesPerSide + x].ix, LandPatches[y * iNumPatchesPerSide + x].iy, 0.0f));
        //         LandPatches[y * iNumPatchesPerSide + x].fDistance = d;
        //         fDistance += d;
        //         if (d < min_distance)
        //         {
        //             min_distance = d;
        //         }
        //         if (d > max_distance)
        //         {
        //             max_distance = d;
        //         }
        //     }
        // }
        // fDistance /= (iNumPatchesPerSide * iNumPatchesPerSide);
        // std::cout << "average distance:" << fDistance << std::endl;
        // std::cout << "min distance:" << min_distance << std::endl;
        // std::cout << "max distance:" << max_distance << std::endl;

        // float distance_sep = (max_distance - min_distance) / (iMaxLOD + 1);

        // 初始 Lod。中间区域最小 lod;
        // int i_lod = 0;
        // int i_ = 0.02 / resolution;
        // while (i_ > 1)
        // {
        //     i_ = i_ >> 1;
        //     i_lod++;
        // }

        for (int32_t y = 0; y < iNumPatchesPerSide; y++)
        {
            for (int32_t x = 0; x < iNumPatchesPerSide; x++)
            {
                // 绑定 VAO
                glBindVertexArray(LandPatches[y * iNumPatchesPerSide + x].VAO);
                // 绘制补丁

                float d = glm::distance(glm::vec3(eye_position.x, eye_position.y, 0), glm::vec3(LandPatches[y * iNumPatchesPerSide + x].ix, LandPatches[y * iNumPatchesPerSide + x].iy, 0.0f));
                // float targetDistance = glm::distance(target, glm::vec3(LandPatches[y * iNumPatchesPerSide + x].ix, LandPatches[y * iNumPatchesPerSide + x].iy, 0.0f));
                // LandPatches[y * iNumPatchesPerSide + x].fDistance = d;
                // float lod_distance = d * resolution; // 计算当前补丁的距离

                // if (lod_distance > 2.0f)
                // {
                //     continue;
                // }
                // int lod = (d * resolution) / 0.5 + i_lod;
                int lod =  d / 300;

                // std::cout << "lod:" << lod << std::endl;
                if (lod > iMaxLOD)
                {
                    continue;
                    lod = iMaxLOD;
                }
                // int lod=0;
                LandPatches[y * iNumPatchesPerSide + x].iLOD = lod;
                int size = LandPatchIndices[lod].indices_count;
                for (int32_t i = 0; i < size; i++)
                {
                    // 绑定索引缓冲区
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, LandPatchIndices[lod].indices[i]);
                    // 绘制补丁
                    glDrawElements(GL_TRIANGLE_FAN, 10, GL_UNSIGNED_INT, 0);
                }
            }
        }
    }

    int m_iSize;

public:
    LandScapeMap(int m_iSize, int iPatchSize)
    {
        this->iPatchSize = iPatchSize;
        this->m_iSize = m_iSize;
        iNumPatchesPerSide = m_iSize / (iPatchSize - 1);
        LandPatches = new LandPatch[iNumPatchesPerSide * iNumPatchesPerSide];
    }
};

class Mesh
{
public:
    Mesh(std::vector<float> vertices, std::vector<unsigned int> indices)
    {
        Init(vertices, indices);
    }

    void Init(std::vector<float> vertices, std::vector<unsigned int> indices)
    {
        // this->vertices = vertices;
        // this->indices = indices;
        // indices_count = indices.size();
        // // 生成 VAO、VBO 和 EBO

        // glGenVertexArrays(1, &VAO);
        // glGenBuffers(1, &VBO);
        // glGenBuffers(1, &EBO);

        // glBindVertexArray(VAO);
        // glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * indices.size(), indices.data(), GL_STATIC_DRAW);

        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        // glEnableVertexAttribArray(0);

        // glBindBuffer(GL_ARRAY_BUFFER, 0);
        // glBindVertexArray(0);
    }

    void Init_VertexBuffer(std::vector<float> vertices)
    {
        this->vertices = vertices;
        // 生成 VAO、VBO

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void Add_IndexBuffer(std::vector<unsigned int> indices)
    {
        this->vertices = vertices;
        indices_count = indices.size();
        // 生成 VAO、VBO 和 EBO
        unsigned int EBO;
        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * indices.size(), indices.data(), GL_STATIC_DRAW);
        EBOS.push_back(EBO);
    }

    Mesh(float CX, float CY, int PatchSize)
    {
        std::vector<float> patch_vertices;
        std::vector<unsigned int> patch_indices;

        int iPatchCount = PatchSize / 2;
        int iLOD = 0;
        int iDivisor = PatchSize - 1;
        while (iDivisor > 2)
        {
            iDivisor = iDivisor >> 1;
            iLOD++;
        }
        for (int j = 0; j < PatchSize; j++)
        {
            for (int i = 0; i < PatchSize; i++)
            {
                patch_vertices.push_back(CX + i);
                patch_vertices.push_back(CY + j);
                patch_vertices.push_back(0.0f); // Z 坐标为 0
            }
        }
        Init_VertexBuffer(patch_vertices);
        // 生成索引缓冲区
        // lod=0; 时;
        for (size_t i = 0; i < iPatchCount; i++)
        {
            for (size_t j = 0; j < iPatchCount; j++)
            {
                int step = 1;
                patch_indices.clear();
                // Render Fan;
                patch_indices.push_back((j * (step * 2) + 1 * step) * PatchSize + (i * (step * 2) + 1 * step));
                patch_indices.push_back((j * (step * 2) + 2 * step) * PatchSize + i * (step * 2));
                patch_indices.push_back((j * (step * 2) + 1 * step) * PatchSize + i * (step * 2));
                patch_indices.push_back((j * (step * 2)) * PatchSize + i * (step * 2));
                patch_indices.push_back((j * (step * 2)) * PatchSize + (i * (step * 2) + 1 * step));
                patch_indices.push_back((j * (step * 2)) * PatchSize + (i * (step * 2) + 2 * step));

                patch_indices.push_back((j * (step * 2) + 1 * step) * PatchSize + (i * (step * 2) + 2 * step));
                patch_indices.push_back((j * (step * 2) + 2 * step) * PatchSize + (i * (step * 2) + 2 * step));
                patch_indices.push_back((j * (step * 2) + 2 * step) * PatchSize + (i * (step * 2) + 1 * step));
                patch_indices.push_back((j * (step * 2) + 2 * step) * PatchSize + i * (step * 2));
                Add_IndexBuffer(patch_indices);
            }
        }
    }

    ~Mesh()
    {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }

    unsigned int getVAO() const { return VAO; }
    std::vector<unsigned int> getEBOS() const { return EBOS; }
    int getIndicesCount() const { return indices_count; }

private:
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int indices_count;
    unsigned int VAO, VBO;

    std::vector<unsigned int> EBOS;
};

// 顶点着色器
const char *vertexShaderSource = R"(
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
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.0, 1.0, 1.0); // 白色
}
)";

// 相机类
class Camera
{
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
    float zoom;

    Camera(glm::vec3 startPosition, glm::vec3 startUp, float startYaw, float startPitch)
        : position(startPosition), worldUp(startUp), yaw(startYaw), pitch(startPitch), speed(2.5f), sensitivity(0.1f), zoom(45.0f)
    {
        updateCameraVectors();
    }

    glm::mat4 getViewMatrix()
    {
        return glm::lookAt(position, position + front, up);
    }

    void processMouseMovement(float xOffset, float yOffset)
    {
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        updateCameraVectors();
    }

    void processMouseScroll(float yOffset)
    {
        zoom -= yOffset;
        if (zoom < 1.0f)
            zoom = 1.0f;
        if (zoom > 500.0f)
            zoom = 500.0f;
    }

private:
    void updateCameraVectors()
    {
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
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
bool firstMouse = true;
float lastX = 400, lastY = 300;
bool rightMousePressed = false;

// 鼠标回调函数
void mouseCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (!rightMousePressed)
        return;

    if (firstMouse)
    {
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
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            rightMousePressed = true;
            firstMouse = true; // 重置鼠标初始位置
        }
        else if (action == GLFW_RELEASE)
        {
            rightMousePressed = false;
        }
    }
}

// 鼠标滚轮回调函数
void scrollCallback(GLFWwindow *window, double xOffset, double yOffset)
{
    // 根据滚轮的方向调整相机位置
    float cameraSpeed = static_cast<float>(yOffset) * 0.5f; // 调整速度
    camera.position += camera.front * cameraSpeed;
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// 高度图分辨率
int m_iSize; // the size of the heightmap, must be a power of two

int main()
{

    // CGEOMIPMAPPING terrain;
    // terrain.m_iSize=257;

    // terrain.Init(17);

    // terrain.Render();

    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Camera with Mouse Control", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
                       {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (key == GLFW_KEY_Q)
            camera.position += camera.up * camera.speed * 0.5f; 
        if (key == GLFW_KEY_E)
            camera.position -= camera.up * camera.speed * 0.5f;
        if (key == GLFW_KEY_W)
            camera.position += camera.front * camera.speed * 0.5f;
        if (key == GLFW_KEY_S)
            camera.position -= camera.front * camera.speed * 0.5f;
        if (key == GLFW_KEY_A)
            camera.position -= camera.right * camera.speed * 0.5f;
        if (key == GLFW_KEY_D)
            camera.position += camera.right * camera.speed * 0.5f; });
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Mesh mesh(b_vertices, b_indices);
    LandScapeMap landScapeMap(8193, 65);
    landScapeMap.init();

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

        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), 800.0f / 600.0f, 0.1f, 10000.0f);
        glm::mat4 view = camera.getViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // float l = camera.position.z / camera.front.z;
        // glm::vec4 p_o = projection * view * glm::vec4(0, 0, 0, 1.0f);
        // p_o = p_o / p_o.w;
        // glm::vec4 p_o1 = projection * view * glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
        // p_o1 = p_o1 / p_o1.w;
        // // 单位长度像素尺寸，控制像素长度为0.2
        // float px = glm::distance(glm::vec3(p_o.x, p_o.y, p_o.z), glm::vec3(p_o1.x, p_o1.y, p_o1.z));
        // std::cout << "distance:" << px << std::endl;
        // 控制最小网格密度为 0.02
        landScapeMap.render(camera.position);
        // landScapeMap.render(camera.position);
        //  glBindVertexArray(mesh.getVAO());

        // std::vector<unsigned int> EBOS = mesh.getEBOS();
        // for (size_t i = 0; i < EBOS.size(); i++)
        // {
        //     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBOS[i]);                              // 绑定第一个索引缓冲区
        //     glDrawElements(GL_TRIANGLE_FAN, mesh.getIndicesCount(), GL_UNSIGNED_INT, 0); // 绘制三角形
        // }

        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // 解绑索引缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}