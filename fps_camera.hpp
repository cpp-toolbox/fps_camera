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

    bool zoomed_in;
    float original_fov, fov, zoom_fov, near_plane, far_plane;

    double unscoped_sensitivity;
    double active_sensitivity;

    FPSCamera(glm::vec3 start_position = glm::vec3(0.0f, 0.0f, 0.0f), double user_sensitivity = 1.0,
              float fov = 90.0f,         // Field of view in degrees
              float zoom_fov = 30.0f,    // Field of view in degrees
              float near_plane = 0.01f,  // Near clipping plane
              float far_plane = 200.0f); // Far clipping plane

    void toggle_zoom();

    void change_active_sensitivity(double new_sens);
    void zoom_in();
    void zoom_out();

    void process_input(bool slow_move_pressed, bool fast_move_pressed, bool forward_pressed, bool left_pressed,
                       bool backward_pressed, bool right_pressed, bool up_pressed, bool down_pressed, float delta_time);

    void mouse_callback(double xpos, double ypos);

    glm::mat4 get_view_matrix() const;
    glm::mat4 get_view_matrix_at(glm::vec3 position) const;
    glm::mat4 get_projection_matrix(const unsigned int screen_width_px, const unsigned int screen_height_px) const;
};

#endif // FPS_CAMERA_HPP
