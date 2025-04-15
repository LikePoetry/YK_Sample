#include <glad/glad.h>
#include <glfw/glfw3.h>

#include <iostream>
#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

int main()
{
    GLFWwindow *window;
    if (!glfwInit())
    {
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);

    window = glfwCreateWindow(650, 480, "Sample001", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // OPENGL 部分
    const char *vertexShaderSource = R"(#version 450
                                        #extension GL_ARB_separate_shader_objects : enable
                                        #extension GL_ARB_shading_language_420pack : enable
                                        #extension GL_KHR_vulkan_glsl : enable
                                     layout(location = 0) out vec2 outTexCoord;
                                     out gl_PerVertex
                                     {
                                         vec4 gl_Position;
                                     };

                                     void main()
                                     {
                                         vec2 position = vec2(gl_VertexID % 2, gl_VertexID / 2) * 4.0 - 1;
                                         outTexCoord = position * 0.5 + 0.5;
                                         gl_Position = vec4(position, 0.0, 1.0);
                                     })";

    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    const char *fragmentShaderSource = R"(#version 450
                                        #extension GL_ARB_separate_shader_objects : enable
                                        #extension GL_ARB_shading_language_420pack : enable
                                        #extension GL_KHR_vulkan_glsl : enable
                                        layout(location = 0) in vec2 outTexCoord;
                                       layout(location = 0) out vec4 outColor;
                                       void main()
                                       {
                                          outColor = vec4(outTexCoord,0.0f,1.0f);
                                       })";

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // unsigned int VAO;
    // glGenVertexArrays(1, &VAO);

    // glBindVertexArray(0);

    while (!glfwWindowShouldClose(window))
    {

        glfwPollEvents();
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        // 在渲染循环中调用 glVertexAttribPointer
        //glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glfwSwapBuffers(window);
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}