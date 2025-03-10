cmake_minimum_required(VERSION 3.10)
project(imgui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 设置静态运行时库
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

# 收集 imgui 文件夹下的所有 .cpp 和 .h 文件
file(GLOB IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
)

# 创建静态库 imgui
add_library(imgui STATIC ${IMGUI_SOURCES})

# 包含头文件目录
target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 设置编译选项
target_compile_options(imgui PRIVATE $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>)
target_compile_options(imgui PRIVATE $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Werror>)