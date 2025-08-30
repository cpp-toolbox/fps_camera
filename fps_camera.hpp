#ifndef FPS_CAMERA_HPP
#define FPS_CAMERA_HPP

#include "sbpt_generated_includes.hpp"

#include <functional>

// NOTE: we define a frustum by the area enclosed by 6 planes
struct Frustum {
    std::array<linalg_utils::Plane, 6> planes;

    // TODO: need to figure out why this function works at all...
    template <typename Container> bool intersects_points(const Container &points) const {
        for (const auto &plane : planes) {
            bool all_outside = true;

            for (const auto &p : points) {
                auto side = plane.classify_point(p);
                bool inside = side == linalg_utils::Plane::Side::NormalSide;
                all_outside &= !inside;
            }

            if (all_outside) {
                return false;
            }
        }
        return true;
    }
};

struct ICamera {
    virtual ~ICamera() = default;
    virtual glm::mat4 get_view_matrix() const = 0;
    virtual glm::mat4 get_projection_matrix() const = 0;
    virtual Frustum get_visible_frustum_world_space() = 0;
    Transform transform;
};

class FPSCamera : public ICamera {
  public:
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
    float original_fov, zoom_fov, near_plane, far_plane;

    Observable<float> fov;

    double unscoped_sensitivity;
    double active_sensitivity;

    unsigned int &screen_width_px;
    unsigned int &screen_height_px;

    FPSCamera(unsigned int &screen_width_px, unsigned int &screen_height_px,
              glm::vec3 start_position = glm::vec3(0.0f, 0.0f, 0.0f), double user_sensitivity = 1.0,
              float fov = 90.0f,         // Field of view in degrees
              float zoom_fov = 30.0f,    // Field of view in degrees
              float near_plane = 0.01f,  // Near clipping plane
              float far_plane = 300.0f); // Far clipping plane

    void toggle_zoom();

    void change_active_sensitivity(double new_sens);
    void zoom_in();
    void zoom_out();

    Frustum get_visible_frustum_world_space() override {

        // Camera params
        float aspect = static_cast<float>(screen_width_px) / screen_height_px;
        float fov_y = glm::radians(fov.get());
        float near_z = near_plane;
        float far_z = far_plane;

        float tan_half_fov_y = tanf(fov_y * 0.5f);
        float near_height = 2.0f * near_z * tan_half_fov_y;
        float near_width = near_height * aspect;
        float far_height = 2.0f * far_z * tan_half_fov_y;
        float far_width = far_height * aspect;

        // camera-space corners
        std::array<glm::vec3, 8> cam_corners = {
            glm::vec3(-near_width / 2, -near_height / 2, -near_z), glm::vec3(near_width / 2, -near_height / 2, -near_z),
            glm::vec3(-near_width / 2, near_height / 2, -near_z),  glm::vec3(near_width / 2, near_height / 2, -near_z),
            glm::vec3(-far_width / 2, -far_height / 2, -far_z),    glm::vec3(far_width / 2, -far_height / 2, -far_z),
            glm::vec3(-far_width / 2, far_height / 2, -far_z),     glm::vec3(far_width / 2, far_height / 2, -far_z)};

        // TRS matrices from camera
        glm::vec3 t = transform.get_translation();
        glm::vec3 s = transform.get_scale();
        glm::vec3 r = transform.get_rotation();
        glm::vec3 r_rad = r * glm::two_pi<float>();

        glm::mat4 T = glm::translate(glm::mat4(1.0f), t);
        glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), r_rad.x, glm::vec3(1, 0, 0));
        // TODO: once again I have no idea why we keep having to fix this
        glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), -r_rad.y - glm::two_pi<float>() / 4, glm::vec3(0, 1, 0));
        glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), r_rad.z, glm::vec3(0, 0, 1));
        glm::mat4 R = Ry * Rx * Rz;
        glm::mat4 S = glm::scale(glm::mat4(1.0f), s);
        glm::mat4 world_matrix = T * R * S;

        std::array<glm::vec3, 8> world_corners;
        int i = 0;
        for (auto &c : cam_corners) {
            glm::vec4 w = world_matrix * glm::vec4(c, 1.0f);
            world_corners[i] = glm::vec3(w);
            i++;
        }

        glm::vec3 nbl = world_corners[0], nbr = world_corners[1], ntl = world_corners[2], ntr = world_corners[3];
        glm::vec3 fbl = world_corners[4], fbr = world_corners[5], ftl = world_corners[6], ftr = world_corners[7];

        /*
         *     ftl*--------------------------*ftr
         *        | .                      . |
         *        |    .                .    |
         *        |       .          .       |
                  |      ntl*------*ntr      |
         *        |         |      |         |
         *        |         |      |         |
                  |      nbl*------*nbr      |
         *        |       .          .       |
         *        |    .                .    |
         *        | .                      . |
         *     fbl*--------------------------*fbr
         *
         */
        // NOTE: we use the above rule to make sure that normals are facing inside of the frustum
        auto left = linalg_utils::Plane(nbl, fbl, ftl);   // left
        auto right = linalg_utils::Plane(nbr, ntr, ftr);  // right
        auto bottom = linalg_utils::Plane(nbl, nbr, fbr); // bottom
        auto top = linalg_utils::Plane(ntl, ftl, ftr);    // top
        auto near = linalg_utils::Plane(ntl, ntr, nbr);   // near
        auto far = linalg_utils::Plane(ftr, ftl, fbl);    // far

        return {left, right, bottom, top, near, far};
    }

    void process_input(bool slow_move_pressed, bool fast_move_pressed, bool forward_pressed, bool left_pressed,
                       bool backward_pressed, bool right_pressed, bool up_pressed, bool down_pressed, float delta_time);

    void mouse_callback(double xpos, double ypos, double sensitivity_override = -1);

    // NOTE: this gets the matrix that puts the camera in the right place and looking in the right place
    glm::mat4 get_view_matrix() const override;
    glm::mat4 get_third_person_view_matrix() const;
    glm::mat4 get_view_matrix_at(glm::vec3 position) const;
    // NOTE: this applies the projection to the view matrix
    glm::mat4 get_projection_matrix() const override;
};

#endif // FPS_CAMERA_HPP
