#include "render/camera.hpp"

#include <algorithm>
#include <cmath>

#ifndef GLFW_INCLUDED
    #include <GLFW/glfw3.h>
    #define GLFW_INCLUDED
#endif

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include "physics/phys_obj_view.hpp"

namespace whsim {

namespace {

constexpr float WORLD_UP_Z = 1.0f;
constexpr glm::vec3 WORLD_UP{0.0f, 0.0f, WORLD_UP_Z};

constexpr float PI = 3.14159265358979323846f;
constexpr float MIN_PITCH = -PI * 0.45f;
constexpr float MAX_PITCH = PI * 0.45f;

constexpr float FOLLOW_DISTANCE = 4.5f;
constexpr float FREE_MOVE_SPEED = 5.0f;
constexpr float KEY_LOOK_SPEED = 1.8f;
constexpr float MOUSE_LOOK_SPEED = 0.003f;
constexpr float MAX_FRAME_TIME = 0.1f;

bool KeyPressed(GLFWwindow* window, int key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool LeftMousePressed(GLFWwindow* window)
{
    return glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

glm::vec3 PositionFromPhysObj(const PhysObj& object)
{
    return glm::vec3{
        static_cast<float>(object.obj_matrix[12]),
        static_cast<float>(object.obj_matrix[13]),
        static_cast<float>(object.obj_matrix[14])
    };
}

bool TryFindWheelTarget(
    const std::vector<PhysObj>& objects,
    glm::vec3& target)
{
    for (const PhysObj& object : objects) {
        if (object.kind == PhysObjKind::Wheel) {
            target = PositionFromPhysObj(object);
            return true;
        }
    }

    return false;
}

} // namespace

void Camera::Update(
    GLFWwindow* window,
    const std::vector<PhysObj>& objects,
    float dt,
    bool input_enabled)
{
    if (window == nullptr) {
        return;
    }

    const float frame_time = std::clamp(dt, 0.0f, MAX_FRAME_TIME);

    RefreshFollowTarget(objects);

    if (!input_enabled) {
        f2_pressed_ = KeyPressed(window, GLFW_KEY_F2);
        right_mouse_pressed_ = false;

        if (mode_ == CameraMode::FollowWheel) {
            position_ = FollowPosition();
        }

        return;
    }

    HandleModeSwitch(window);
    HandleLookInput(window, frame_time);

    if (mode_ == CameraMode::Free) {
        HandleFreeMovement(window, frame_time);
    } else {
        position_ = FollowPosition();
    }
}

glm::mat4 Camera::ViewMatrix() const
{
    if (mode_ == CameraMode::FollowWheel) {
        return glm::lookAt(FollowPosition(), follow_target_, WORLD_UP);
    }

    return glm::lookAt(position_, position_ + Forward(), WORLD_UP);
}

glm::mat4 Camera::ProjectionMatrix(int width, int height) const
{
    const float safe_width = static_cast<float>(std::max(1, width));
    const float safe_height = static_cast<float>(std::max(1, height));

    return glm::perspective(
        glm::radians(50.0f),
        safe_width / safe_height,
        0.05f,
        200.0f
    );
}

CameraMode Camera::Mode() const noexcept
{
    return mode_;
}

void Camera::HandleModeSwitch(GLFWwindow* window)
{
    const bool f2_pressed = KeyPressed(window, GLFW_KEY_F2);

    if (f2_pressed && !f2_pressed_) {
        if (mode_ == CameraMode::FollowWheel) {
            SetMode(CameraMode::Free);
        } else {
            SetMode(CameraMode::FollowWheel);
        }
    }

    f2_pressed_ = f2_pressed;
}

void Camera::HandleLookInput(GLFWwindow* window, float dt)
{
    if (KeyPressed(window, GLFW_KEY_LEFT)) {
        yaw_ += KEY_LOOK_SPEED * dt;
    }

    if (KeyPressed(window, GLFW_KEY_RIGHT)) {
        yaw_ -= KEY_LOOK_SPEED * dt;
    }

    if (KeyPressed(window, GLFW_KEY_UP)) {
        pitch_ += KEY_LOOK_SPEED * dt;
    }

    if (KeyPressed(window, GLFW_KEY_DOWN)) {
        pitch_ -= KEY_LOOK_SPEED * dt;
    }

    if (LeftMousePressed(window)) {
        double mouse_x = 0.0;
        double mouse_y = 0.0;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);

        if (right_mouse_pressed_) {
            const auto delta_x = static_cast<float>(mouse_x - previous_mouse_x_);
            const auto delta_y = static_cast<float>(mouse_y - previous_mouse_y_);

            yaw_ -= delta_x * MOUSE_LOOK_SPEED;
            pitch_ -= delta_y * MOUSE_LOOK_SPEED;
        }

        previous_mouse_x_ = mouse_x;
        previous_mouse_y_ = mouse_y;
        right_mouse_pressed_ = true;
    } else {
        right_mouse_pressed_ = false;
    }

    pitch_ = std::clamp(pitch_, MIN_PITCH, MAX_PITCH);
}

void Camera::HandleFreeMovement(GLFWwindow* window, float dt)
{
    glm::vec3 movement{0.0f};
    const glm::vec3 forward = PlanarForward();
    const glm::vec3 right = Right();

    if (KeyPressed(window, GLFW_KEY_W)) {
        movement += forward;
    }

    if (KeyPressed(window, GLFW_KEY_S)) {
        movement -= forward;
    }

    if (KeyPressed(window, GLFW_KEY_D)) {
        movement += right;
    }

    if (KeyPressed(window, GLFW_KEY_A)) {
        movement -= right;
    }

    if (KeyPressed(window, GLFW_KEY_R)) {
        movement += WORLD_UP;
    }

    if (KeyPressed(window, GLFW_KEY_F)) {
        movement -= WORLD_UP;
    }

    if (glm::dot(movement, movement) > 0.0f) {
        position_ += glm::normalize(movement) * FREE_MOVE_SPEED * dt;
    }
}

void Camera::RefreshFollowTarget(const std::vector<PhysObj>& objects)
{
    glm::vec3 target = follow_target_;

    if (TryFindWheelTarget(objects, target)) {
        follow_target_ = target;
    }
}

void Camera::SetMode(CameraMode mode)
{
    if (mode_ == mode) {
        return;
    }

    position_ = CurrentPosition();
    mode_ = mode;
}

glm::vec3 Camera::Forward() const
{
    const float cos_pitch = std::cos(pitch_);

    return glm::normalize(glm::vec3{
        std::cos(yaw_) * cos_pitch,
        std::sin(yaw_) * cos_pitch,
        std::sin(pitch_)
    });
}

glm::vec3 Camera::PlanarForward() const
{
    const glm::vec3 forward = Forward();
    const glm::vec3 planar_forward{forward.x, forward.y, 0.0f};

    if (glm::dot(planar_forward, planar_forward) <= 0.000001f) {
        return glm::vec3{0.0f, 1.0f, 0.0f};
    }

    return glm::normalize(planar_forward);
}

glm::vec3 Camera::Right() const
{
    return glm::normalize(glm::cross(PlanarForward(), WORLD_UP));
}

glm::vec3 Camera::FollowPosition() const
{
    return follow_target_ - Forward() * FOLLOW_DISTANCE;
}

glm::vec3 Camera::CurrentPosition() const
{
    if (mode_ == CameraMode::FollowWheel) {
        return FollowPosition();
    }

    return position_;
}

} // namespace whsim
