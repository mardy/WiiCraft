#pragma once
#include "math.hpp"
#include "gfx.hpp"
#include "ext/data_array.hpp"
#include <vector>
#include <optional>
#include <functional>
#include <map>

namespace game {
    struct camera {
        glm::vec3 position;
        glm::vec3 up;
        glm::vec3 look;
        f32 yaw = 0.0f;
        f32 pitch = 0.0f;
        f32 fov;
        f32 aspect;
        f32 near_clipping_plane_distance;
        f32 far_clipping_plane_distance;
    };

    struct camera_update_params {
        bool update_view = false;
        bool update_look = false;
	    bool update_perspective = false;
    };

    void update_view(const camera& cam, math::matrix view);
    void update_look(camera& cam);

    void move_camera(camera& cam, const glm::vec3& input_vector, f32 move_speed);
    void rotate_camera(camera& cam, const glm::vec2& input_vector, f32 rotate_speed);
    

    inline void update_perspective(const camera& cam, math::matrix44 perspective) {
        guPerspective(perspective, cam.fov, cam.aspect, cam.near_clipping_plane_distance, cam.far_clipping_plane_distance);
    }

    struct block {
        enum class face : u8 {
            FRONT, // +x
            BACK, // -x
            RIGHT, // +z
            LEFT, // -z
            TOP, // +y
            BOTTOM, // -z
            CENTER
        };
        enum class type : u8 {
            AIR,
            DEBUG,
            GRASS,
            DIRT
        };
        type tp;
        // This is a cache of which faces the block needs to create face vertices for.
        struct face_cache {
            bool front;
            bool back;
            bool right;
            bool left;
            bool top;
            bool bottom;
        };
    };

    template<
        block::face face,
        typename F0,
        typename F1,
        typename F2,
        typename F3,
        typename F4,
        typename F5,
        typename F6,
        typename ...A
    >
    constexpr auto call_face_func_if(F0 front, F1 back, F2 right, F3 left, F4 top, F5 bottom, F6 center, A&&... args) {
        if constexpr (face == block::face::FRONT) {
            return front(std::forward<A>(args)...);
        } else if constexpr (face == block::face::BACK) {
            return back(std::forward<A>(args)...);
        } else if constexpr (face == block::face::RIGHT) {
            return right(std::forward<A>(args)...);
        } else if constexpr (face == block::face::LEFT) {
            return left(std::forward<A>(args)...);
        } else if constexpr (face == block::face::TOP) {
            return top(std::forward<A>(args)...);
        } else if constexpr (face == block::face::BOTTOM) {
            return bottom(std::forward<A>(args)...);
        } else if constexpr (face == block::face::CENTER) {
            return center(std::forward<A>(args)...);
        }
    }

    bool is_block_visible(block::type type);

    template<block::face face, typename T>
    constexpr T get_face_offset_position(T pos) {
        call_face_func_if<face>(
            [&pos]() { pos.x += 1; },
            [&pos]() { pos.x -= 1; },
            [&pos]() { pos.z += 1; },
            [&pos]() { pos.z -= 1; },
            [&pos]() { pos.y += 1; },
            [&pos]() { pos.y -= 1; },
            []() {}
        );
        return pos;
    }

    struct chunk {
        using map = std::unordered_map<math::vector3s32, chunk>;
        using opt_ref = std::optional<std::reference_wrapper<chunk>>;
        using const_opt_ref = std::optional<std::reference_wrapper<const chunk>>;

        static constexpr s32 SIZE = 32;
        static constexpr u32 BLOCKS_COUNT = SIZE * SIZE * SIZE;
        
        struct mesh {
            struct vertex {
                math::vector3u8 local_position;
                math::vector2u8 uv_position;
            };
            ext::data_array<vertex> vertices;
        };

        mesh ms;

        struct neighborhood {
            opt_ref front;
            opt_ref back;
            opt_ref right;
            opt_ref left;
            opt_ref top;
            opt_ref bottom;
        };

        neighborhood nh;
        bool update_mesh = true;
        ext::data_array<block> blocks = { BLOCKS_COUNT };

        math::matrix model;
        math::matrix model_view;
    };

    chunk::neighborhood get_chunk_neighborhood(chunk::map& chunks, const math::vector3s32& position);

    template<block::face face>
    constexpr chunk::const_opt_ref get_neighbor(const chunk::neighborhood& nh) {
        if constexpr (face == block::face::FRONT) {
            return nh.front;
        } else if constexpr (face == block::face::BACK) {
            return nh.back;
        } else if constexpr (face == block::face::RIGHT) {
            return nh.right;
        } else if constexpr (face == block::face::LEFT) {
            return nh.left;
        } else if constexpr (face == block::face::TOP) {
            return nh.top;
        } else if constexpr (face == block::face::BOTTOM) {
            return nh.bottom;
        }
    }

    template<block::face face>
    constexpr chunk::opt_ref get_neighbor(chunk::neighborhood& nh) {
        if constexpr (face == block::face::FRONT) {
            return nh.front;
        } else if constexpr (face == block::face::BACK) {
            return nh.back;
        } else if constexpr (face == block::face::RIGHT) {
            return nh.right;
        } else if constexpr (face == block::face::LEFT) {
            return nh.left;
        } else if constexpr (face == block::face::TOP) {
            return nh.top;
        } else if constexpr (face == block::face::BOTTOM) {
            return nh.bottom;
        }
    }

    inline void update_model_view(chunk& chunk, math::matrix view) {
        guMtxConcat(view, chunk.model, chunk.model_view);
    }

    math::vector3u8 get_position_from_index(std::size_t index);
    template<typename T>
    inline std::size_t get_index_from_position(T position) {
        return position.x + (position.y * chunk::SIZE) + (position.z * chunk::SIZE * chunk::SIZE);
    }
    inline s32 get_world_coord_from_local_position(s32 local_coord, s32 chunk_coord) {
        return ((chunk_coord * chunk::SIZE) + local_coord);
    }
    inline math::vector3s32 floor_float_position(const glm::vec3& position) {
        return {
            floorf(position.x),
            floorf(position.y),
            floorf(position.z)
        };
    }
    inline math::vector3s32 get_chunk_position_from_world_position(glm::vec3 world_position) {
        return floor_float_position(world_position / (f32)chunk::SIZE);
    }
    void init(chunk& chunk, const math::vector3s32& chunk_position, math::matrix view);

    template<typename F>
    void iterate_over_chunk_positions(F func) {
        for (u8 x = 0; x < chunk::SIZE; x++) {
            for (u8 y = 0; y < chunk::SIZE; y++) {
                for (u8 z = 0; z < chunk::SIZE; z++) {
                    func({x, y, z});
                }
            }
        }
    }

    template<typename B, typename F>
    void iterate_over_chunk_blocks_and_positions(B& blocks, F func) {
        for (u8 x = 0; x < chunk::SIZE; x++) {
            for (u8 y = 0; y < chunk::SIZE; y++) {
                for (u8 z = 0; z < chunk::SIZE; z++) {
                    math::vector3u8 pos = {x, y, z};
                    func(blocks[get_index_from_position(pos)], pos);
                }
            }
        }
    }

    template<typename B, typename F>
    void iterate_over_chunk_positions_and_blocks(B& blocks, F func) {
        for (u8 x = 0; x < chunk::SIZE; x++) {
            for (u8 y = 0; y < chunk::SIZE; y++) {
                for (u8 z = 0; z < chunk::SIZE; z++) {
                    math::vector3u8 pos = {x, y, z};
                    func(pos, blocks[get_index_from_position(pos)]);
                }
            }
        }
    }

    void generate_blocks(chunk& chunk, const math::vector3s32& pos, u32 seed);

    void update_mesh(chunk& chunk, ext::data_array<game::block::face_cache>& face_caches);

    void add_chunk_mesh_update(chunk& chunk, math::vector3u8 block_position);
    template<block::face face>
    void add_chunk_mesh_update_to_neighbor(chunk& chunk) {
        auto nb_chunk_opt = get_neighbor<face>(chunk.nh);
        if (nb_chunk_opt.has_value()) {
            auto& nb_chunk = nb_chunk_opt->get();
            nb_chunk.update_mesh = true;
        }
    }

    void draw_chunk_mesh_vertices(const ext::data_array<chunk::mesh::vertex>& vertices);
    void draw_chunk(chunk& chunk);

    std::size_t get_center_vertex_count(block::type type);
    std::size_t get_any_face_vertex_count(block::type type);

    template<block::face face>
    constexpr std::size_t get_face_vertex_count(block::type type) {
        return call_face_func_if<face>(
            get_any_face_vertex_count,
            get_any_face_vertex_count,
            get_any_face_vertex_count,
            get_any_face_vertex_count,
            get_any_face_vertex_count,
            get_any_face_vertex_count,
            get_center_vertex_count,
            type
        );
    }

    using vertex_it = ext::data_array<chunk::mesh::vertex>::iterator;

    void add_front_vertices(vertex_it& it, math::vector3u8 local_position, block::type type);
    void add_back_vertices(vertex_it& it, math::vector3u8 local_position, block::type type);
    void add_right_vertices(vertex_it& it, math::vector3u8 local_position, block::type type);
    void add_left_vertices(vertex_it& it, math::vector3u8 local_position, block::type type);
    void add_top_vertices(vertex_it& it, math::vector3u8 local_position, block::type type);
    void add_bottom_vertices(vertex_it& it, math::vector3u8 local_position, block::type type);
    void add_center_vertices(vertex_it& it, math::vector3u8 local_position, block::type type);

    template<block::face face>
    constexpr void add_face_vertices(vertex_it& it, math::vector3u8 local_position, block::type type) {
        call_face_func_if<face>(
            add_front_vertices,
            add_back_vertices,
            add_right_vertices,
            add_left_vertices,
            add_top_vertices,
            add_bottom_vertices,
            add_center_vertices,
            it, local_position, type
        );
    }

    void destroy_block_from_camera(const camera& cam, chunk::map& chunks);
}