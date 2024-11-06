#ifndef FPS_CAMERA_HPP
#define FPS_CAMERA_HPP

#include "sbpt_generated_includes.hpp"
#include <GLFW/glfw3.h>

class FPSCamera {
  public:
    Transform transform;
    Mouse mouse;

    float move_speed = 5;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::mat4 projection; // Projection matrix

    // Constructor
    FPSCamera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f), double user_sensitivity = 1.0,
              float screen_width = 800.0f, float screen_height = 600.0f,
              float fov = 45.0f,        // Field of view (default: 45 degrees)
              float near_plane = 0.01f, // Near clipping plane (default: 0.01f)
              float far_plane = 50.0f); // Far clipping plane (default: 50.0f)

    // Processes keyboard input for movement
    void process_input(GLFWwindow *window, float deltaTime);

    // Processes mouse input for camera rotation using the Mouse class
    void mouse_callback(double xpos, double ypos);

    // Returns the view matrix
    glm::mat4 get_view_matrix() const;

    // Returns the projection matrix
    glm::mat4 get_projection_matrix() const;
};

#endif // FPS_CAMERA_HPP
