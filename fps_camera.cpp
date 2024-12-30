#include "fps_camera.hpp"

#include <GLFW/glfw3.h>
#include <iostream> // For std::cout in mouse_callback

// Constructor
FPSCamera::FPSCamera(glm::vec3 startPosition, double user_sensitivity, float screen_width, float screen_height,
                     float fov, float near_plane, float far_plane)
    : mouse(user_sensitivity) {
    transform.position = startPosition; // Set initial camera position

    // Calculate the projection matrix using the provided parameters
    projection = glm::perspective(glm::radians(fov), screen_width / screen_height, near_plane, far_plane);
}

// Processes keyboard input for movement
void FPSCamera::process_input(GLFWwindow *window, float delta_time) {

    float selected_speed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        selected_speed = slow_move_speed;
    } else {
        selected_speed = move_speed;
    }

    float delta_pos = selected_speed * delta_time;

    // Compute forward and right vectors
    glm::vec3 forward = transform.compute_forward_vector();
    glm::vec3 right = glm::normalize(glm::cross(forward, up)); // Up vector is (0, 1, 0)

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        transform.position += delta_pos * forward; // Move forward
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        transform.position -= delta_pos * forward; // Move backward
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        transform.position -= delta_pos * right; // Move left
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        transform.position += delta_pos * right; // Move right
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        transform.position += delta_pos * up; // Move forward
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        transform.position -= delta_pos * up; // Move forward
    }
}

// Processes mouse input for camera rotation using the Mouse class
void FPSCamera::mouse_callback(double xpos, double ypos) {
    // Get the yaw and pitch deltas from the Mouse class
    auto [yaw_delta, pitch_delta] = mouse.get_yaw_pitch_deltas(xpos, ypos);

    transform.rotation.y -= yaw_delta;   // Yaw
    transform.rotation.x += pitch_delta; // Pitch

    // Clamp the pitch
    if (transform.rotation.x > 89.0f)
        transform.rotation.x = 89.0f;
    if (transform.rotation.x < -89.0f)
        transform.rotation.x = -89.0f;
}

// Returns the view matrix
glm::mat4 FPSCamera::get_view_matrix() const {
    return glm::lookAt(transform.position, transform.position + transform.compute_forward_vector(),
                       glm::vec3(0.0f, 1.0f, 0.0f));
}

// Returns the projection matrix
glm::mat4 FPSCamera::get_projection_matrix() const { return projection; }
