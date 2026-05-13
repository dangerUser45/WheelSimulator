cmake_minimum_required(VERSION 3.20)
project(WheelSimulator LANGUAGES C CXX)

# Enabling building compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SANITIZE "Enable sanitizers" OFF)

find_package(Bullet REQUIRED CONFIG)
find_package(Eigen3 REQUIRED CONFIG)
find_package(glad   REQUIRED CONFIG)
find_package(glfw3  REQUIRED CONFIG)
find_package(glm    REQUIRED CONFIG)
find_package(imgui  REQUIRED CONFIG)
find_package(implot REQUIRED CONFIG)
find_package(OpenGL REQUIRED)

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
    
    src/control/application.cpp
    src/control/sim_controller.cpp
    src/control/ui_controller.cpp
    src/control/window_controller.cpp
    
    src/physics/physics.cpp
    src/physics/phys_obj_view.cpp
    src/physics/terrain_grid.cpp
    
    src/render/camera.cpp
    src/render/mesh.cpp
    src/render/sim_render.cpp
    src/render/ui_impl.cpp
    src/render/ui_render.cpp
    
    3rdparty/stb/src/stb_load_image.cpp
)

target_compile_features(WheelSimulator PRIVATE cxx_std_23)

target_include_directories(WheelSimulator PRIVATE
    include
    3rdparty/stb/include
)

target_link_libraries(WheelSimulator PRIVATE
    Bullet::Bullet
    Eigen3::Eigen
    glad::glad
    glfw
    glm::glm
    imgui::imgui
    imgui_backends
    implot::implot
    OpenGL::GL
)

if(MSVC)
    target_compile_options(WheelSimulator PRIVATE
        /W4
        /permissive-
    )
else()
    target_compile_options(WheelSimulator PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Weffc++
    )
endif()

target_compile_definitions(WheelSimulator PRIVATE
    PRESETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/preset/"
    SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src/shaders/"
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
