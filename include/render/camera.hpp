#pragma once

#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include "config/simulation_settings.hpp"

struct GLFWwindow;

namespace whsim {

struct PhysObj;

class Camera final {
private:
    CameraSettings settings_{};
    CameraMode mode_{};

    glm::vec3 position_{};
    glm::vec3 follow_target_{};

    float yaw_ = 0.0f;
    float pitch_ = 0.0f;

    bool f2_pressed_ = false;
    bool right_mouse_pressed_ = false;

    double previous_mouse_x_ = 0.0;
    double previous_mouse_y_ = 0.0;

private:
    void HandleModeSwitch(GLFWwindow* window);
    void HandleLookInput(GLFWwindow* window, float dt);
    void HandleFreeMovement(GLFWwindow* window, float dt);
    void RefreshFollowTarget(const std::vector<PhysObj>& objects);
    void SetMode(CameraMode mode);

    [[nodiscard]] glm::vec3 Forward() const;
    [[nodiscard]] glm::vec3 PlanarForward() const;
    [[nodiscard]] glm::vec3 Right() const;
    [[nodiscard]] glm::vec3 FollowPosition() const;
    [[nodiscard]] glm::vec3 CurrentPosition() const;

public:
    Camera();

    void ApplySettings(const CameraSettings& settings);

    void Update(
        GLFWwindow* window,
        const std::vector<PhysObj>& objects,
        float dt,
        bool input_enabled);

    [[nodiscard]] glm::mat4 ViewMatrix() const;
    [[nodiscard]] glm::mat4 ProjectionMatrix(int width, int height) const;
    [[nodiscard]] CameraMode Mode() const noexcept;
};

} // namespace whsim
