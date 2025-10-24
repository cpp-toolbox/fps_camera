#ifndef FPS_CAMERA_HPP
#define FPS_CAMERA_HPP

#include "sbpt_generated_includes.hpp"

#include <functional>
#include <glm/ext/matrix_transform.hpp>

// NOTE: we define a frustum by the area enclosed by 6 planes
struct Frustum {
    std::array<linalg_utils::Plane, 6> planes;

    // TODO: need to figure out why this function works at all...
    // this is mainly used for frustum culling
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

// NOTE: if we ever need to redo this then I want to redo it by using connect n-gon from vertex geom.
// draw_info::IndexedVertexPositions generate_frustum_ivp(bool center_at_origin = false) {
//     // Camera params
//     float aspect = static_cast<float>(screen_width_px) / screen_height_px;
//     float fov_y = glm::radians(camera.fov.get()); // vertical FOV
//     float near_z = camera.near_plane;
//     float far_z = camera.far_plane;
//     far_z = 2;
//
//     // Half-sizes of near/far planes
//     float tan_half_fov_y = tanf(fov_y * 0.5f);
//     float near_height = 2.0f * near_z * tan_half_fov_y;
//     float near_width = near_height * aspect;
//     float far_height = 2.0f * far_z * tan_half_fov_y;
//     float far_width = far_height * aspect;
//
//     // Define frustum corners in camera space
//     std::array<glm::vec3, 8> cam_corners = {
//         // Near plane
//         glm::vec3(-near_width / 2, -near_height / 2, -near_z), glm::vec3(near_width / 2, -near_height / 2,
//         -near_z), glm::vec3(-near_width / 2, near_height / 2, -near_z), glm::vec3(near_width / 2, near_height /
//         2, -near_z),
//         // Far plane
//         glm::vec3(-far_width / 2, -far_height / 2, -far_z), glm::vec3(far_width / 2, -far_height / 2, -far_z),
//         glm::vec3(-far_width / 2, far_height / 2, -far_z), glm::vec3(far_width / 2, far_height / 2, -far_z)};
//
//     std::vector<glm::vec3> world_corners;
//     world_corners.reserve(8);
//
//     // --- Manually construct TRS matrix from Transform ---
//     glm::vec3 t = camera.transform.get_translation();
//     glm::vec3 s = camera.transform.get_scale();
//     glm::vec3 r = camera.transform.get_rotation(); // in turns (0..1)
//     glm::vec3 r_rad = r * glm::two_pi<float>();    // convert turns to radians
//
//     // Translation matrix
//     glm::mat4 T = glm::translate(glm::mat4(1.0f), t);
//
//     // Rotation matrices (Euler order: pitch=X, yaw=Y, roll=Z)
//     glm::mat4 Rx = glm::rotate(glm::mat4(1.0f), r_rad.x, glm::vec3(1, 0, 0));
//     glm::mat4 Ry = glm::rotate(glm::mat4(1.0f), -r_rad.y - glm::two_pi<float>() / 4, glm::vec3(0, 1, 0));
//     glm::mat4 Rz = glm::rotate(glm::mat4(1.0f), r_rad.z, glm::vec3(0, 0, 1));
//     glm::mat4 R = Ry * Rx * Rz; // assuming YXZ order (common for FPS camera)
//
//     // Scale matrix
//     glm::mat4 S = glm::scale(glm::mat4(1.0f), s);
//
//     // Combine to world matrix
//     glm::mat4 world_matrix = T * R * S; // TRS order
//
//     for (auto &c : cam_corners) {
//         glm::vec4 world = world_matrix * glm::vec4(c, 1.0f);
//         world_corners.push_back(glm::vec3(world));
//     }
//
//     if (center_at_origin) {
//         for (auto &p : world_corners) {
//             p -= t; // shift apex to origin
//         }
//     }
//
//     // Triangle indices
//     std::vector<unsigned int> indices = {
//         0, 1, 2, 1, 3, 2, // near
//         4, 6, 5, 5, 6, 7, // far
//         0, 2, 4, 2, 6, 4, // left
//         1, 5, 3, 3, 5, 7, // right
//         2, 3, 6, 3, 7, 6, // top
//         0, 4, 1, 1, 4, 5  // bottom
//     };
//
//     return draw_info::IndexedVertexPositions(indices, world_corners);
// }

inline std::array<glm::vec3, 8> get_aabb_corners_world(const vertex_geometry::AxisAlignedBoundingBox &box,
                                                       Transform &transform) {
    glm::mat4 model = transform.get_transform_matrix();
    std::array<glm::vec3, 8> corners = box.get_corners();
    for (auto &c : corners) {
        glm::vec4 world = model * glm::vec4(c, 1.0f);
        c = glm::vec3(world);
    }
    return corners;
}

struct ICamera {
    virtual ~ICamera() = default;
    virtual glm::mat4 get_view_matrix() const = 0;
    virtual glm::mat4 get_projection_matrix() const = 0;
    // TODO: remove the below one
    virtual Frustum get_visible_frustum_world_space() = 0;
    virtual bool is_visible(const std::vector<glm::vec3> &xyz_positions, Transform &transform) = 0;
    virtual bool is_visible(const vertex_geometry::AxisAlignedBoundingBox &aabb, Transform &transform) = 0;
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
    void toggle_camera_freeze();
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
        // NOTE: on windows I get a compliation error because apparently near and far is a keyword so I'll append an
        // underscore to fix this.
        auto near_ = linalg_utils::Plane(ntl, ntr, nbr); // near
        auto far_ = linalg_utils::Plane(ftr, ftl, fbl);  // far

        return {left, right, bottom, top, near_, far_};
    }

    bool is_visible(const std::vector<glm::vec3> &xyz_positions, Transform &transform) override {
        LogSection _(global_logger, "is_visible");
        auto local_aabb = vertex_geometry::AxisAlignedBoundingBox(xyz_positions);
        return is_visible(local_aabb, transform);
    }

    bool is_visible(const vertex_geometry::AxisAlignedBoundingBox &aabb, Transform &transform) override {
        LogSection _(global_logger, "is_visible");
        auto frustum = get_visible_frustum_world_space();
        auto corners = get_aabb_corners_world(aabb, transform);
        return frustum.intersects_points(corners);
    }

    void process_input(bool slow_move_pressed, bool fast_move_pressed, bool forward_pressed, bool left_pressed,
                       bool backward_pressed, bool right_pressed, bool up_pressed, bool down_pressed, float delta_time);

    void mouse_callback(double xpos, double ypos, double sensitivity_override = -1);

    /**
     * @brief this is the matrix that puts the camera in the right place and looking in the right place
     */
    glm::mat4 get_view_matrix() const override;
    glm::mat4 get_third_person_view_matrix() const;
    glm::mat4 get_view_matrix_at(glm::vec3 position) const;
    /**
     * @brief the projection matrix is the one that takes the world and applies the perspective view on the world
     */
    glm::mat4 get_projection_matrix() const override;
};

struct Camera2D : ICamera {
    float zoom = 1.0f;

    glm::vec2 offset = glm::vec2(0);

    double last_mouse_pos_x = 0.0f;
    double last_mouse_pos_y = 0.0f;

    bool is_dragging = false;

    unsigned int &screen_width_px, &screen_height_px;

    Camera2D(unsigned int &screen_width_px, unsigned int &screen_height_px)
        : screen_width_px(screen_width_px), screen_height_px(screen_height_px) {}

    Frustum get_visible_frustum_world_space() override {
        float aspect = 1.0f; // caller can set actual aspect
        float view_width = zoom * aspect;
        float view_height = zoom;

        // corners of the ortho box
        glm::vec3 nbl(-view_width + offset.x, -view_height + offset.y, -1.0f);
        glm::vec3 nbr(view_width + offset.x, -view_height + offset.y, -1.0f);
        glm::vec3 ntl(-view_width + offset.x, view_height + offset.y, -1.0f);
        glm::vec3 ntr(view_width + offset.x, view_height + offset.y, -1.0f);

        glm::vec3 fbl(-view_width + offset.x, -view_height + offset.y, 1.0f);
        glm::vec3 fbr(view_width + offset.x, -view_height + offset.y, 1.0f);
        glm::vec3 ftl(-view_width + offset.x, view_height + offset.y, 1.0f);
        glm::vec3 ftr(view_width + offset.x, view_height + offset.y, 1.0f);

        // build planes just like in 3D
        auto left = linalg_utils::Plane(nbl, fbl, ftl);
        auto right = linalg_utils::Plane(nbr, ntr, ftr);
        auto bottom = linalg_utils::Plane(nbl, nbr, fbr);
        auto top = linalg_utils::Plane(ntl, ftl, ftr);
        // NOTE: on windows I get a compliation error because apparently near and far is a keyword so I'll append an
        // underscore to fix this.
        auto near_ = linalg_utils::Plane(ntl, ntr, nbr);
        auto far_ = linalg_utils::Plane(ftr, ftl, fbl);

        return {left, right, bottom, top, near_, far_};
    }

    glm::mat4 get_view_matrix() const override {
        // because we bake position info into projection which is weird fix later
        return glm::mat4(1);
    }

    bool is_visible(const std::vector<glm::vec3> &xyz_positions, Transform &transform) override {
        LogSection _(global_logger, "is_visible");

        // compute object aabb in local space
        auto local_aabb = vertex_geometry::AxisAlignedBoundingBox(xyz_positions);

        return is_visible(local_aabb, transform);
    }

    bool is_visible(const vertex_geometry::AxisAlignedBoundingBox &aabb, Transform &transform) override {
        LogSection _(global_logger, "is_visible");

        auto corners = aabb.get_corners();

        // Transform corners into world space
        glm::mat4 model = transform.get_transform_matrix();
        glm::vec3 min_world(std::numeric_limits<float>::max());
        glm::vec3 max_world(-std::numeric_limits<float>::max());
        for (auto &c : corners) {
            glm::vec4 world_c = model * glm::vec4(c, 1.0f);
            min_world = glm::min(min_world, glm::vec3(world_c));
            max_world = glm::max(max_world, glm::vec3(world_c));
        }

        // Now we have a world-space AABB
        glm::vec2 aabb_min(min_world.x, min_world.y);
        glm::vec2 aabb_max(max_world.x, max_world.y);

        // Compute camera view rectangle
        float aspect = static_cast<float>(screen_width_px) / screen_height_px;
        float view_width = zoom * aspect;
        float view_height = zoom;

        glm::vec2 cam_min(-view_width + offset.x, -view_height + offset.y);
        glm::vec2 cam_max(view_width + offset.x, view_height + offset.y);

        // Check overlap (2D AABB vs AABB)
        return !(aabb_max.x < cam_min.x || aabb_min.x > cam_max.x || aabb_max.y < cam_min.y || aabb_min.y > cam_max.y);
    }

    // NOTE: yeah this is kinda dumb, fix it up when it matters
    glm::mat4 get_projection_matrix() const override { return get_transform_matrix(); }

    glm::mat4 get_transform_matrix() const {
        float aspect = static_cast<float>(screen_width_px) / screen_height_px;
        float view_width = zoom * aspect;
        float view_height = zoom;

        glm::mat4 projection = glm::ortho(-view_width + offset.x, view_width + offset.x, -view_height + offset.y,
                                          view_height + offset.y, -1.0f, 1.0f);
        return projection;
    }

    void on_scroll(double x_offset, double y_offset) {
        if (y_offset > 0) {
            zoom /= 1.1;
        } else {
            zoom *= 1.1;
        }
    }

    // TODO: one day I want to implement that "momentum style of dragging"
    void update(double mouse_delta_x, double mouse_delta_y, unsigned int width, unsigned int height, bool is_dragging) {
        if (is_dragging) {
            float aspect = static_cast<float>(width) / height;
            float sensitivity = 10;
            offset.x -= sensitivity * static_cast<float>(mouse_delta_x) / width * zoom * aspect * 2.0f;
            offset.y += sensitivity * static_cast<float>(mouse_delta_y) / height * zoom * 2.0f;
        }
    }
};

#endif // FPS_CAMERA_HPP
