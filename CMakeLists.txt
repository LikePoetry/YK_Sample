cmake_minimum_required(VERSION 3.10.0)
project(yk VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

# 定义 GLM_ENABLE_EXPERIMENTAL 宏
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# 添加 GLFW 库
add_subdirectory(extern/glfw)

# 添加 imgui 库
add_subdirectory(extern/imgui)

# 添加 libtiff 库路径
set(LIBTIFF_LIB_PATH "${CMAKE_SOURCE_DIR}/extern/libtiff/lib")
set(LIBTIFF_INCLUDE_PATH "${CMAKE_SOURCE_DIR}/extern/libtiff/include")

# 添加可执行文件
add_executable(YK main.cpp geomipmapping.cpp geomipmapping.h terrain.cpp terrain.h extern/glad/src/glad.c stb_image.h imgui_impl_opengl3_loader.h imgui_impl_opengl3.h imgui_impl_opengl3.cpp imgui_impl_glfw.h imgui_impl_glfw.cpp)

# 链接 logsystem、ykengine、spdlog、GLFW 和 GLAD 库到可执行文件
target_link_libraries(YK PRIVATE glfw imgui "${LIBTIFF_LIB_PATH}/tiff.lib")
target_include_directories(YK PRIVATE extern/glfw/include extern/glad/include "${LIBTIFF_INCLUDE_PATH}" extern/glm extern extern/imgui)