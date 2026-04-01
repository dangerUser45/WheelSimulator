cmake_minimum_required(VERSION 3.20)
project(WheelSimulator LANGUAGES C CXX)

# Enabling building compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SANITIZE "Enable sanitizers" OFF)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Eigen3 REQUIRED NO_MODULE)

#-------------------------------------------------------------------------------
# Setup imgui lib
#-------------------------------------------------------------------------------
add_library(imgui STATIC
    3rdparty/imgui/imgui.cpp
    3rdparty/imgui/imgui_demo.cpp
    3rdparty/imgui/imgui_draw.cpp
    3rdparty/imgui/imgui_tables.cpp
    3rdparty/imgui/imgui_widgets.cpp
    3rdparty/imgui/backends/imgui_impl_glfw.cpp
    3rdparty/imgui/backends/imgui_impl_opengl3.cpp
 )

target_include_directories(imgui PUBLIC
    3rdparty/imgui
    3rdparty/imgui/backends
)

target_link_libraries(imgui PUBLIC
    OpenGL::GL
    glfw
)

#-------------------------------------------------------------------------------
# Setup implot lib
#-------------------------------------------------------------------------------
add_library(implot STATIC
    3rdparty/implot/implot.cpp
    3rdparty/implot/implot_items.cpp
    3rdparty/implot/implot_demo.cpp
 )

target_include_directories(implot PUBLIC
    3rdparty/implot
)

target_link_libraries(implot PUBLIC
    imgui
)

#-------------------------------------------------------------------------------
# Setup glad lib
#-------------------------------------------------------------------------------
add_library(glad STATIC
    3rdparty/glad/src/gl.c
)

target_include_directories(glad PUBLIC
    3rdparty/glad/include
)

#-------------------------------------------------------------------------------
# Setup wheel simulator
#-------------------------------------------------------------------------------
add_executable(WheelSimulator
    src/main.cpp
    src/application.cpp
)

target_compile_features(WheelSimulator PRIVATE cxx_std_23)

target_include_directories(WheelSimulator PRIVATE
    include
)

target_link_libraries(WheelSimulator PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    Eigen3::Eigen

    glad
    imgui
    implot
)

target_compile_options(WheelSimulator PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    # -Weffc++
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
