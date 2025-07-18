#include "game.h"

#include <core/logger.h>
#include <core/input.h>
#include <core/tmemory.h>
#include <core/event.h>

#include <math/tmath.h>


// HACK: this should not be available outside the engine.
#include <renderer/renderer_frontend.h>

void recalculate_view_matrix(game_state* state){
    if(state->camera_view_dirty){
        mat4 rotation = mat4_euler_xyz(state->camera_euler.x, state->camera_euler.y, state->camera_euler.z);
        mat4 translation = mat4_translation(state->camera_position);

        state->view = mat4_mul(rotation, translation);
        state->view = mat4_inverse(state->view);

        state->camera_view_dirty = FALSE;
    }
}

void camera_yaw(game_state* state, f32 amount){
    state->camera_euler.y += amount;
    state->camera_view_dirty = TRUE;
}

void camera_pitch(game_state* state, f32 amount){
    state->camera_euler.x += amount;

    // Clamp to avoid Gimball lock.
    f32 limit = deg_to_rad(89.0f);
    state->camera_euler.x = TCLAMP(state->camera_euler.x, -limit, limit);

    state->camera_view_dirty = TRUE;
}

b8 game_initialize(game* game_inst){
    TDEBUG("game_initialize() called!");

    game_state* state = (game_state*)game_inst->state;

    state->camera_position = (vec3){10.5f, 5.0f, 9.5f};
    state->camera_euler = vec3_zero();

    state->view = mat4_translation(state->camera_position);
    state->view = mat4_inverse(state->view);
    state->camera_view_dirty = TRUE;

    return TRUE;
}

b8 game_update(game* game_inst, f32 delta_time){
    
    static u64 alloc_count = 0;
    u64 prev_alloc_count = alloc_count;
    alloc_count = get_memory_alloc_count();
    if(input_is_key_up('M') && input_was_key_down('M')){
        TDEBUG("Allocations: %llu (%llu this frame)", alloc_count, alloc_count - prev_alloc_count);
    }
    
    // TODO: temporary
    if(input_is_key_up('T') && input_was_key_down('T')){
        TDEBUG("Swapping texture!");
        event_context context = {};
        event_fire(EVENT_CODE_DEBUG0, game_inst, context);
    }
    // TODO: end temporary

    game_state* state = (game_state*)game_inst->state;

    // HACK: this should not be available outside the engine.
    if(input_is_key_down('A') || input_is_key_down(KEY_LEFT)){
        camera_yaw(state, 1.0f * delta_time);
    }
    // HACK: this should not be available outside the engine.
    if(input_is_key_down('D') || input_is_key_down(KEY_RIGHT)){
        camera_yaw(state, -1.0f * delta_time);
    }
    // HACK: this should not be available outside the engine.
    if(input_is_key_down(KEY_UP)){
        camera_pitch(state, 1.0f * delta_time);
    }
    // HACK: this should not be available outside the engine.
    if(input_is_key_down(KEY_DOWN)){
        camera_pitch(state, -1.0f * delta_time);
    }
    f32 temp_move_speed = 50.0f;
    vec3 velocity = vec3_zero();
     // HACK: this should not be available outside the engine.
    if(input_is_key_down('W')){
        vec3 forward = mat4_forward(state->view);
        velocity = vec3_add(velocity, forward);
    }
    // HACK: this should not be available outside the engine.
    if(input_is_key_down('S')){
        vec3 backward = mat4_backward(state->view);
        velocity = vec3_add(velocity, backward);
    }
    // HACK: this should not be available outside the engine.
    if(input_is_key_down('Q')){
        vec3 left = mat4_left(state->view);
        velocity = vec3_add(velocity, left);
    }
    // HACK: this should not be available outside the engine.
    if(input_is_key_down('E')){
        vec3 right = mat4_right(state->view);
        velocity = vec3_add(velocity, right);
    }
        // HACK: this should not be available outside the engine.
    if(input_is_key_down(KEY_SPACE)){
        velocity.y += 1.0f;
    }
    // HACK: this should not be available outside the engine.
    if(input_is_key_down('X')){
        velocity.y -= 1.0f;
    }

    vec3 z = vec3_zero();
    if(!vec3_compare(z, velocity, 0.0002f)){
        // Be sure to normalize the velocity before applying speed.
        vec3_normalize(&velocity);
        state->camera_position.x += velocity.x * temp_move_speed * delta_time;
        state->camera_position.y += velocity.y * temp_move_speed * delta_time;
        state->camera_position.z += velocity.z * temp_move_speed * delta_time;
        state->camera_view_dirty = TRUE;
    }

    recalculate_view_matrix(state);

    // HACK: this should not be available outside the engine.
    renderer_set_view(state->view, state->camera_position);

    // TODO: temporary
    if(input_is_key_up('P') && input_was_key_down('P')){
        TDEBUG(
            "Pos: [%.2f, %.2f, %.2f]",
            state->camera_position.x,
            state->camera_position.y,
            state->camera_position.z
        );
    }

    // RENDERER DEBUG FUNCTIONS
    if(input_is_key_up('1') && input_was_key_down('1')){
        event_context data = {};
        data.data.i32[0] = RENDERER_VIEW_MODE_LIGHTING;
        event_fire(EVENT_CODE_SET_RENDER_MODE, game_inst, data);
    }

    if(input_is_key_up('2') && input_was_key_down('2')){
        event_context data = {};
        data.data.i32[0] = RENDERER_VIEW_MODE_NORMALS;
        event_fire(EVENT_CODE_SET_RENDER_MODE, game_inst, data);
    }

    if(input_is_key_up('0') && input_was_key_down('0')){
        event_context data = {};
        data.data.i32[0] = RENDERER_VIEW_MODE_DEFAULT;
        event_fire(EVENT_CODE_SET_RENDER_MODE, game_inst, data);
    }

    // TODO: end temporary 

    return TRUE;
}

b8 game_render(game* game_inst, f32 delta_time){
    return TRUE;
}

void game_on_resize(game* game_inst, u32 width, u32 heigth){

}