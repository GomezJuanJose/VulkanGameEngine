#include "renderer_frontend.h"

#include "renderer_backend.h"

#include "core/logger.h"
#include "core/tmemory.h"

#include "math/tmath.h"

#include "resources/resource_types.h"
#include "systems/texture_system.h"
#include "systems/material_system.h"

// TODO: temporary
#include "core/tstring.h"
#include "core/event.h"
// TODO: end temporary

typedef struct renderer_system_state {
    renderer_backend backend;
    mat4 projection;
    f32 near_clip;
    f32 far_clip;

    mat4 view;


} renderer_system_state;


// Backend render context.
static renderer_system_state* state_ptr;

b8 renderer_system_initialize(u64* memory_requirement, void* state, const char* application_name){
    *memory_requirement = sizeof(renderer_system_state);
    if(state == 0){
        return TRUE;
    }
    state_ptr = state;


    // TODO: make this configurable
    renderer_backend_create(RENDERER_BACKEND_TYPE_VULKAN, &state_ptr->backend);
    state_ptr->backend.frame_number = 0;

    if(!state_ptr->backend.initialize(&state_ptr->backend, application_name)){
        TFATAL("Renderer backend failed to initialized. Shutting down...");
        return FALSE;
    }


    state_ptr->near_clip = 0.1f;
    state_ptr->far_clip = 1000.0f;
    state_ptr->projection = mat4_perspective(deg_to_rad(45.0f),  1280 /720.0f, state_ptr->near_clip, state_ptr->far_clip);

    state_ptr->view = mat4_translation((vec3){0, 0, 30.0f});
    state_ptr->view = mat4_inverse(state_ptr->view);

    return TRUE;
}

void renderer_system_shutdown(){
    if(state_ptr){
        state_ptr->backend.shutdown(&state_ptr->backend);
    }

    state_ptr = 0;
}

b8 renderer_begin_frame(f32 delta_time){
    if(!state_ptr){
        return FALSE;
    }
    return state_ptr->backend.begin_frame(&state_ptr->backend, delta_time);
}

b8 renderer_end_frame(f32 delta_time){
    if (!state_ptr){
        return FALSE;
    }

    b8 result = state_ptr->backend.end_frame(&state_ptr->backend, delta_time);
    state_ptr->backend.frame_number++;
    return result;
}

void renderer_on_resized(u16 width, u16 height){
    if(state_ptr){
        state_ptr->projection = mat4_perspective(deg_to_rad(45.0f), width / (f32)height, state_ptr->near_clip, state_ptr->far_clip);
        state_ptr->backend.resized(&state_ptr->backend, width, height);
    }else{
        TWARN("renderer backend does not exist to accept resize: %i %i", width, height);
    }
}

b8 renderer_draw_frame(render_packet* packet){
    // If the begin frame returned successfully, mid-frame operations may continue.
    if(renderer_begin_frame(packet->delta_time)){

        state_ptr->backend.update_global_state(state_ptr->projection, state_ptr->view, vec3_zero(), vec4_one(), 0);

        u32 count = packet->geometry_count;
        for(u32 i = 0; i < count; ++i){
            state_ptr->backend.draw_geometry(packet->geometries[i]);
        }

        // End the frame. If this fails, it is likely unrecoverable.
        b8 result = renderer_end_frame(packet->delta_time);

        if(!result){
            TERROR("renderer_end_frame failed. Application shutting down...");
            return FALSE;
        }
    }

    return TRUE;
}

void renderer_set_view(mat4 view){
    state_ptr->view = view;
}

void renderer_create_texture(const u8* pixels, struct texture* texture){
    state_ptr->backend.create_texture(pixels, texture);
}

void renderer_destroy_texture(struct texture* texture){
    state_ptr->backend.destroy_texture(texture);
}

b8 renderer_create_material(struct material* material){
    return state_ptr->backend.create_material(material);
}

void renderer_destroy_material(struct material* material){
    state_ptr->backend.destroy_material(material);
}

b8 renderer_create_geometry(geometry* geometry, u32 vertex_count, const vertex_3d* vertices, u32 index_count, const u32* indices){
    return state_ptr->backend.create_geometry(geometry, vertex_count, vertices, index_count, indices);
}

void renderer_destroy_geometry(geometry* geometry){
    state_ptr->backend.destroy_geometry(geometry);
}