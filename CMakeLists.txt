cmake_minimum_required(VERSION 3.20)

project(bit)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# Vulkan
find_package(Vulkan REQUIRED)


# GLFW
set(GLFW_ROOT "${CMAKE_SOURCE_DIR}/modules/GLFW")

add_library(glfw STATIC IMPORTED)

set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION
        "${GLFW_ROOT}/lib-vc2026/glfw3.lib"

    INTERFACE_INCLUDE_DIRECTORIES
        "${GLFW_ROOT}/include"
)


# Bit
add_executable(bit

    src/main.cpp

    src/files/file_manager.cpp

    src/api/Apis.cpp
    src/api/Execute.cpp

    src/services/Clock.cpp

    src/vulcan/ex.cpp
    src/vulcan/GetGPUinfo.cpp
    src/vulcan/RenderPipe.cpp
    src/vulcan/ex.hpp
    src/Clock/clock.cpp
)


# Include directories
target_include_directories(bit PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/modules"
    "${CMAKE_SOURCE_DIR}/web"
)


# Definitions
target_compile_definitions(bit PRIVATE
    WEB_ROOT="${CMAKE_SOURCE_DIR}/web"
    API_ROOT="${CMAKE_SOURCE_DIR}/src/config"
)


# Libraries
target_link_libraries(bit PRIVATE
    Vulkan::Vulkan
    glfw
)