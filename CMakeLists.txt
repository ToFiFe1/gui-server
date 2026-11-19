
cmake_minimum_required(VERSION 3.20)

project(BIT LANGUAGES CXX)

# ==================================================
# Allgemeine Einstellungen
# ==================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ==================================================
# Vulkan - Windows und Linux
# ==================================================

find_package(Vulkan REQUIRED)

# ==================================================
# GLFW - plattformabhängig finden
# ==================================================

if(WIN32)

    # Windows: vorhandene GLFW-Bibliothek
    set(GLFW_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/modules/GLFW")
    set(GLFW_LIBRARY "${GLFW_ROOT}/lib-vc2026/glfw3.lib")

    if(NOT EXISTS "${GLFW_LIBRARY}")
        message(FATAL_ERROR
            "GLFW Windows-Bibliothek nicht gefunden: ${GLFW_LIBRARY}"
        )
    endif()

    add_library(BIT_GLFW STATIC IMPORTED)

    set_target_properties(BIT_GLFW PROPERTIES
        IMPORTED_LOCATION "${GLFW_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_ROOT}/include"
    )

elseif(UNIX)

    # Linux: native GLFW-Installation
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLFW REQUIRED IMPORTED_TARGET glfw3)

    add_library(BIT_GLFW INTERFACE)
    target_link_libraries(BIT_GLFW INTERFACE PkgConfig::GLFW)

else()

    message(FATAL_ERROR "Dieses Betriebssystem wird nicht unterstützt.")

endif()

# ==================================================
# BIT - Quelldateien
# ==================================================

add_executable(bit
    src/main.cpp

    src/files/file_manager.cpp

    src/api/Apis.cpp
    src/api/Execute.cpp

    src/services/Clock.cpp

    src/vulcan/ex.cpp
    src/vulcan/GetGPUinfo.cpp
    src/vulcan/RenderPipe.cpp

    src/Clock/clock.cpp
)

# ==================================================
# Include-Verzeichnisse
# ==================================================

target_include_directories(bit PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/modules"
    "${CMAKE_CURRENT_SOURCE_DIR}/web"
)

# GLFW-Header auf beiden Plattformen verfügbar machen
if(WIN32)
    target_include_directories(bit PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/modules/GLFW/include"
    )
endif()

# ==================================================
# Projektpfade
# ==================================================

target_compile_definitions(bit PRIVATE
    "WEB_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}/web\""
    "API_ROOT=\"${CMAKE_CURRENT_SOURCE_DIR}/src/config\""
)

# ==================================================
# Bibliotheken verknüpfen
# ==================================================

target_link_libraries(bit PRIVATE
    Vulkan::Vulkan
    BIT_GLFW
)

# ==================================================
# Build-Informationen
# ==================================================

message(STATUS "BIT Betriebssystem: ${CMAKE_SYSTEM_NAME}")
message(STATUS "BIT C++-Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "BIT Vulkan: ${Vulkan_FOUND}")
message(STATUS "BIT GLFW: konfiguriert")
