#pragma once
#include "block_raycast.hpp"
#include "logic.hpp"
#include "math.hpp"
#include "mesh_generation.hpp"
#include "ext/data_array.hpp"
#include "gfx/display_list.hpp"
#include "math/transform_3d.hpp"
#include <optional>

namespace game {
    struct block_selection {
        std::optional<math::vector3u8> last_block_pos;
        std::optional<block> last_block;

	    gfx::display_list standard_disp_list;
	    gfx::display_list foliage_disp_list;
	    gfx::display_list water_disp_list;
        math::transform_3d tf;

        void update_if_needed(const math::matrix view, const camera& cam);
        void draw_standard(const std::optional<block_raycast>& raycast) const;
        void draw_foliage(const std::optional<block_raycast>& raycast) const;
        void draw_water(const std::optional<block_raycast>& raycast) const;
        void handle_raycast(const math::matrix view, standard_quad_building_arrays& building_arrays, const std::optional<block_raycast>& raycast);

        private:
            void update_mesh(const math::matrix view, standard_quad_building_arrays& building_arrays, const block_raycast& raycast);
    };
}