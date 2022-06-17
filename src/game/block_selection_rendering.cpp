#include "block_selection_rendering.hpp"

using namespace game;

void game::init_block_selection_attrs() {
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);

	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_U8, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
}

void game::draw_block_selection(block_selection& bl_sel) {
    gfx::load(bl_sel.pos_state);

	bl_sel.disp_list.call();
}