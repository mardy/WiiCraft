#include "input.hpp"

using namespace input;

void input::init(u32 width, u32 height) {
    WPAD_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL, WPAD_FMT_BTNS_ACC_IR);
    WPAD_SetVRes(WPAD_CHAN_ALL, width, height);
}

math::vector3f input::get_dpad_input_vector(u32 buttons_held) {
    math::vector3f pad_input_vector = {0.0f, 0.0f, 0.0f};
    if (buttons_held & WPAD_BUTTON_UP) {
        pad_input_vector += { 1, 0, 0 };
    }
    if (buttons_held & WPAD_BUTTON_DOWN) {
        pad_input_vector -= { 1, 0, 0 };
    }
    if (buttons_held & WPAD_BUTTON_LEFT) {
        pad_input_vector += { 0, 0, 1 };
    }
    if (buttons_held & WPAD_BUTTON_RIGHT) {
        pad_input_vector -= { 0, 0, 1 };
    }
    if (buttons_held & WPAD_BUTTON_PLUS) {
        pad_input_vector += { 0, 1, 0 };
    }
    if (buttons_held & WPAD_BUTTON_MINUS) {
        pad_input_vector -= { 0, 1, 0 };
    }
    return pad_input_vector;
}

math::vector2f input::get_pointer_input_vector(state& s, u32 buttons_held) {
    ir_t pointer;
    WPAD_IR(0, &pointer);
    if (pointer.valid) {
        math::vector2f pointer_pos = {pointer.sx, pointer.sy};
        if ((buttons_held & WPAD_BUTTON_A) && s.was_last_pointer_pos_valid && pointer_pos != s.last_pointer_pos) {
            math::vector2f pointer_input_vector = pointer_pos - s.last_pointer_pos;
            s.was_last_pointer_pos_valid = true;
            s.last_pointer_pos = pointer_pos;
            return pointer_input_vector;
        }
        s.was_last_pointer_pos_valid = true;
        s.last_pointer_pos = pointer_pos;
    } else {
        s.was_last_pointer_pos_valid = false;
    }
    return {0.0f, 0.0f};
}