#pragma once

#include <cstdint>
#include <vector>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct GLFWwindow;

namespace whsim {

struct PhysObj;

enum class CameraMode : std::uint8_t {
    FollowWheel,
    Free
};

class Camera final {
private:
    CameraMode mode_ = CameraMode::FollowWheel;

    glm::vec3 position_{0.0f, -4.5f, 2.0f};
    glm::vec3 follow_target_{0.0f, 0.0f, 0.35f};

    float yaw_ = 1.57079632679f;
    float pitch_ = -0.35f;

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
