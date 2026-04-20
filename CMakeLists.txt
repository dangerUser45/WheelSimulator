cmake_minimum_required(VERSION 3.20)
project(WheelSimulator LANGUAGES C CXX)

# Enabling building compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SANITIZE "Enable sanitizers" OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3  REQUIRED CONFIG)
find_package(glm    REQUIRED CONFIG)
find_package(Eigen3 REQUIRED CONFIG)
find_package(imgui  REQUIRED CONFIG)
find_package(implot REQUIRED CONFIG)
find_package(glad   REQUIRED CONFIG)

#-------------------------------------------------------------------------------
# Setup imgui/backends
#-------------------------------------------------------------------------------
add_library(imgui_backends STATIC
    3rdparty/imgui_backends/src/imgui_impl_glfw.cpp
    3rdparty/imgui_backends/src/imgui_impl_opengl3.cpp
)

target_include_directories(imgui_backends PUBLIC
    3rdparty/imgui_backends/include
)

target_link_libraries(imgui_backends PUBLIC
    imgui::imgui
    glfw
    glad::glad
    OpenGL::GL
)

#-------------------------------------------------------------------------------
# Setup wheel simulator
#-------------------------------------------------------------------------------
add_executable(WheelSimulator
    src/main.cpp
    src/application.cpp
    src/view.cpp
    src/ui.cpp
    3rdparty/stb/src/stb_load_image.cpp
)

target_compile_features(WheelSimulator PRIVATE cxx_std_20)

target_include_directories(WheelSimulator PRIVATE
    include
    3rdparty/stb/include
)

target_link_libraries(WheelSimulator PRIVATE
    Eigen3::Eigen
    glad::glad
    glfw
    glm::glm
    imgui::imgui
    imgui_backends
    implot::implot
    OpenGL::GL
)

target_compile_options(WheelSimulator PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Weffc++
)

target_compile_definitions(WheelSimulator PRIVATE
    PRESETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/presets/"
)

#-------------------------------------------------------------------------------
# Optionally enabling sanitazers for debug 
#-------------------------------------------------------------------------------
if(SANITIZE)
    target_compile_options(WheelSimulator PRIVATE
        -fsanitize=address,undefined
    )

    target_link_options(WheelSimulator PRIVATE
        -fsanitize=address,undefined
    )
endif()
