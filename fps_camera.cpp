#include "fps_camera.hpp"
#include <cmath> // For std::tan and M_PI

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

FPSCamera::FPSCamera(glm::vec3 start_position, double user_sensitivity, float fov, float zoom_fov, float near_plane,
                     float far_plane)
    : unscoped_sensitivity(user_sensitivity), active_sensitivity(user_sensitivity), mouse(user_sensitivity),
      original_fov(fov), fov(fov), zoom_fov(zoom_fov), near_plane(near_plane), far_plane(far_plane) {
    transform.set_translation(start_position);
}

// NOTE: https://toolbox.cuppajoeman.com/programming/game_dev/fov_and_sensitivity.html
float compute_new_sensitivity(float original_sensitivity, float original_fov, float new_fov) {
    float original_fov_rad = original_fov * (M_PI / 180.0f);
    float new_fov_rad = new_fov * (M_PI / 180.0f);

    float half_original_fov = original_fov_rad / 2.0f;
    float half_new_fov = new_fov_rad / 2.0f;

    float tan_half_original_fov = std::tan(half_original_fov);
    float tan_half_new_fov = std::tan(half_new_fov);

    float new_sensitivity = original_sensitivity * (tan_half_new_fov / tan_half_original_fov);

    return new_sensitivity;
}

void FPSCamera::toggle_zoom() {
    if (zoomed_in) {
        zoom_out();
    } else {
        zoom_in();
    }
}

void FPSCamera::change_active_sensitivity(double new_sens) {
    active_sensitivity = new_sens;
    mouse.user_sensitivity = active_sensitivity;
}

void FPSCamera::zoom_in() {
    zoomed_in = true;
    fov = zoom_fov;
    change_active_sensitivity(compute_new_sensitivity(unscoped_sensitivity, original_fov, zoom_fov));
}
void FPSCamera::zoom_out() {
    zoomed_in = false;
    fov = original_fov;
    change_active_sensitivity(unscoped_sensitivity);
}

void FPSCamera::process_input(bool slow_move_pressed, bool fast_move_pressed, bool forward_pressed, bool left_pressed,
                              bool backward_pressed, bool right_pressed, bool up_pressed, bool down_pressed,
                              float delta_time) {

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

    if (forward_pressed)
        movement += forward;
    if (backward_pressed)
        movement -= forward;
    if (left_pressed)
        movement -= right;
    if (right_pressed)
        movement += right;
    if (up_pressed)
        movement += up;
    if (down_pressed)
        movement -= up;

    if (glm::length(movement) > 0.0f) {
        movement = glm::normalize(movement);
    }

    transform.add_translation(movement * delta_pos);
}

void FPSCamera::toggle_mouse_freeze() { camera_frozen = not camera_frozen; }
void FPSCamera::freeze_camera() { camera_frozen = true; }
void FPSCamera::unfreeze_camera() { camera_frozen = false; }

void FPSCamera::mouse_callback(double xpos, double ypos, double sensitivity_override) {
    auto [yaw_delta, pitch_delta] = mouse.get_yaw_pitch_deltas(xpos, ypos, sensitivity_override);

    if (camera_frozen)
        return;

    transform.add_rotation_yaw(-yaw_delta);    // Yaw
    transform.add_rotation_pitch(pitch_delta); // Pitch
    float epsilon = .0001;
    if (transform.get_rotation().x > .25 - epsilon)
        transform.set_rotation_pitch(.25 - epsilon);
    if (transform.get_rotation().x < -.25 + epsilon)
        transform.set_rotation_pitch(-.25 + epsilon);
}

glm::mat4 FPSCamera::get_view_matrix() const {
    return glm::lookAt(transform.get_translation(), transform.get_translation() + transform.compute_forward_vector(),
                       glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 FPSCamera::get_view_matrix_at(glm::vec3 position) const {
    return glm::lookAt(position, position + transform.compute_forward_vector(), glm::vec3(0.0f, 1.0f, 0.0f));
}

// TODO: there is a bug here when screen_width and screen_height externally so then screen width and screen height are
// out of date

glm::mat4 FPSCamera::get_projection_matrix(const unsigned int screen_width_px,
                                           const unsigned int screen_height_px) const {
    return glm::perspective(glm::radians(fov),
                            static_cast<float>(screen_width_px) / static_cast<float>(screen_height_px), near_plane,
                            far_plane);
}
