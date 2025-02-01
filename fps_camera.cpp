#include "fps_camera.hpp"

FPSCamera::FPSCamera(glm::vec3 start_position, double user_sensitivity, float screen_width, float screen_height,
                     float fov, float near_plane, float far_plane)
    : mouse(user_sensitivity) {
    transform.position = start_position; 
    projection = glm::perspective(glm::radians(fov), screen_width / screen_height, near_plane, far_plane);
}

void FPSCamera::process_input(bool slow_move_pressed, bool fast_move_pressed, bool forward_pressed, bool left_pressed, bool backward_pressed, bool right_pressed, float delta_time) {

    float selected_speed;

    if (slow_move_pressed) {
        selected_speed = slow_move_speed;
    } else if (fast_move_pressed) {
        selected_speed = fast_move_speed;
    } else {
        selected_speed = move_speed;
    }

    float delta_pos = selected_speed * delta_time;

    // Compute forward and right vectors
    glm::vec3 forward = transform.compute_forward_vector();
    glm::vec3 right = glm::normalize(glm::cross(forward, up)); // Up vector is (0, 1, 0)
    
    glm::vec3 movement(0.0f);

    if (forward_pressed) movement += forward;
    if (backward_pressed) movement -= forward;
    if (left_pressed) movement -= right;
    if (right_pressed) movement += right;

    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement);
    }

    transform.position += movement * delta_pos;
} 

void FPSCamera::toggle_mouse_freeze() { camera_frozen = not camera_frozen; }
void FPSCamera::freeze_camera() { camera_frozen = true; }
void FPSCamera::unfreeze_camera() { camera_frozen = false; }

void FPSCamera::mouse_callback(double xpos, double ypos) {
    auto [yaw_delta, pitch_delta] = mouse.get_yaw_pitch_deltas(xpos, ypos);

    if (camera_frozen)
        return;

    transform.rotation.y -= yaw_delta;   // Yaw
    transform.rotation.x += pitch_delta; // Pitch

    if (transform.rotation.x > 89.0f)
        transform.rotation.x = 89.0f;
    if (transform.rotation.x < -89.0f)
        transform.rotation.x = -89.0f;
}

glm::mat4 FPSCamera::get_view_matrix() const {
    return glm::lookAt(transform.position, transform.position + transform.compute_forward_vector(),
                       glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 FPSCamera::get_view_matrix_at(glm::vec3 position) const {
    return glm::lookAt(position, position + transform.compute_forward_vector(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 FPSCamera::get_projection_matrix() const { return projection; }
