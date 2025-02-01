#ifndef FPS_CAMERA_HPP
#define FPS_CAMERA_HPP

#include "sbpt_generated_includes.hpp"

class FPSCamera {
  public:
    Transform transform;
    Mouse mouse;

    // this is how many units per second you will move at
    float move_speed = 2;
    float fast_move_speed = move_speed * 4;
    float slow_move_speed = move_speed * .25;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 projection;

    bool camera_frozen = false;
    void toggle_mouse_freeze();
    void freeze_camera();
    void unfreeze_camera();

    FPSCamera(glm::vec3 start_position = glm::vec3(0.0f, 0.0f, 0.0f), double user_sensitivity = 1.0,
              float screen_width = 800.0f, float screen_height = 600.0f,
              float fov = 90.0f,        // Field of view (default: 45 degrees)
              float near_plane = 0.01f, // Near clipping plane (default: 0.01f)
              float far_plane = 50.0f); // Far clipping plane (default: 50.0f)

    void process_input(bool slow_move_pressed, bool fast_move_pressed, bool forward_pressed, bool left_pressed,
                       bool backward_pressed, bool right_pressed, float delta_time);

    void mouse_callback(double xpos, double ypos);

    glm::mat4 get_view_matrix() const;
    glm::mat4 get_view_matrix_at(glm::vec3 position) const;
    glm::mat4 get_projection_matrix() const;
};

#endif // FPS_CAMERA_HPP
