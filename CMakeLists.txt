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
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
set(IMGUI_PACKAGE_DIR "${imgui_PACKAGE_FOLDER_${BUILD_TYPE_UPPER}}")
set(IMGUI_BINDINGS_DIR "${IMGUI_PACKAGE_DIR}/res/bindings")

add_library(imgui_backends STATIC
    "${IMGUI_BINDINGS_DIR}/imgui_impl_glfw.cpp"
    "${IMGUI_BINDINGS_DIR}/imgui_impl_opengl3.cpp"
)

target_include_directories(imgui_backends PUBLIC
    "${IMGUI_BINDINGS_DIR}"
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
    src/ui_theme.cpp
    src/view.cpp
    src/view_panels.cpp
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
    SETUP_PATH="${CMAKE_CURRENT_SOURCE_DIR}/setup/"
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
