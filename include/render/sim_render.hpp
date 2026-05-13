#pragma once

#include <memory>
#include <vector>

#include "render/camera.hpp"

struct GLFWwindow;

namespace whsim {

class Mesh;
struct PhysObj;

class SimRender final {
private:
    std::unique_ptr<Mesh> terrain_mesh_{};
    std::unique_ptr<Mesh> wheel_mesh_{};
    std::unique_ptr<Mesh> box_mesh_{};

    unsigned int shader_program_ = 0;

    int u_model_location_ = -1;
    int u_view_location_ = -1;
    int u_projection_location_ = -1;
    int u_color_location_ = -1;
    int u_light_dir_location_ = -1;

    unsigned int framebuffer_ = 0;
    unsigned int sim_texture_ = 0;
    unsigned int depth_renderbuffer_ = 0;

    int texture_width_ = 0;
    int texture_height_ = 0;

    Camera camera_{};

    bool initialized_ = false;

private:
    void Initialize();
    void InitializeMeshes();
    void InitializeShader();
    void InitializeUniformLocations();

    void EnsureFramebuffer(int width, int height);
    void DestroyFramebufferResources();
    void CreateFramebufferResources(int width, int height);
    void CreateColorTexture(int width, int height);
    void CreateDepthRenderbuffer(int width, int height);
    void AttachFramebufferResources();
    void ValidateFramebuffer() const;

    void BeginRenderToTexture(int width, int height) const;
    void EndRenderToTexture() const;

    void DrawObjects(
        const std::vector<PhysObj>& objects,
        int width,
        int height) const;

    void DrawObject(const PhysObj& object) const;

public:
    SimRender();
    ~SimRender();

    SimRender(const SimRender&) = delete;
    SimRender& operator=(const SimRender&) = delete;

    SimRender(SimRender&&) = delete;
    SimRender& operator=(SimRender&&) = delete;

    [[nodiscard]] unsigned int Render(
        GLFWwindow* window,
        const std::vector<PhysObj>& objects,
        int width,
        int height,
        float dt,
        bool camera_input_enabled);

    [[nodiscard]] unsigned int SimTexture() const noexcept;
};

} // namespace whsim
