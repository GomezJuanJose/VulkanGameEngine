#include "geometry_system.h"

#include "core/logger.h"
#include "core/tmemory.h"
#include "core/tstring.h"
#include "math/geometry_utils.h"
#include "systems/material_system.h"
#include "renderer/renderer_frontend.h"

typedef struct geometry_reference {
    u64 reference_count;
    geometry geometry;
    b8 auto_release;
} geometry_reference;

typedef struct geometry_system_state{
    geometry_system_config config;

    geometry default_geometry;
    geometry default_2d_geometry;

    // Array of registered meshes.
    geometry_reference* registered_geometries;
} geometry_system_state;

static geometry_system_state* state_ptr = 0;

b8 create_default_geometries(geometry_system_state* state);
b8 create_geometry(geometry_system_state* state, geometry_config config, geometry* g);
void destroy_geometry(geometry_system_state* state, geometry* g);

b8 geometry_system_initialize(u64* memory_requirement, void* state, geometry_system_config config){
    if(config.max_geometry_count == 0){
        TFATAL("geometry_system_initialize - config.max_geometry_count must be > 0.");
        return FALSE;
    }

    // Block of memory will contain state structure, then block for array,
    u64 struct_requirement = sizeof(geometry_system_state);
    u64 array_requirement = sizeof(geometry_reference) * config.max_geometry_count;
    *memory_requirement = struct_requirement + array_requirement;

    if(!state){
        return TRUE;
    }

    state_ptr = state;
    state_ptr->config = config;

    // The array block is after the state. Already allocated, so just set the pointer.
    void* array_block = state + struct_requirement;
    state_ptr->registered_geometries = array_block;

    // Invalidate all geometries in the array.
    u32 count = state_ptr->config.max_geometry_count;
    for(u32 i = 0; i < count; ++i){
        state_ptr->registered_geometries[i].geometry.id = INVALID_ID;
        state_ptr->registered_geometries[i].geometry.internal_id = INVALID_ID;
        state_ptr->registered_geometries[i].geometry.generation = INVALID_ID;
    }

    if(!create_default_geometries(state_ptr)){
        TFATAL("Failed to create default geometries. Application cannot continue.");
        return FALSE;
    }

    return TRUE;
}

void geometry_system_shutdown(void* state){
    // NOTE: nothing to do here.
}

geometry* geometry_system_acquire_by_id(u32 id){
    if(id != INVALID_ID && state_ptr->registered_geometries[id].geometry.id != INVALID_ID){
        state_ptr->registered_geometries[id].reference_count++;
        return &state_ptr->registered_geometries[id].geometry;
    }

    // NOTE: Should return default geometry instead?
    TERROR("geometry_system_acquire_by_id cannot load invalid geometry id. Returning nullptr.");
    return 0;
}

geometry* geometry_system_acquire_from_config(geometry_config config, b8 auto_release){
    geometry* g = 0;
    for(u32 i = 0; i < state_ptr->config.max_geometry_count; ++i){
        if(state_ptr->registered_geometries[i].geometry.id == INVALID_ID){
            // Found empty slot.
            state_ptr->registered_geometries[i].auto_release = auto_release;
            state_ptr->registered_geometries[i].reference_count = 1;
            g = &state_ptr->registered_geometries[i].geometry;
            g->id = i;
            break;
        }
    }

    if(!g){
        TERROR("Unable to obtain free slot for geometry. Adjust configuration to allow more space. Returning nullptr.");
        return 0;
    }

    if(!create_geometry(state_ptr, config, g)){
        TERROR("Failed to create geometry. Returning nullptr.");
        return 0;
    }

    return g;
}

void geometry_system_config_dispose(geometry_config* config){
    if(config){
        if(config->vertices){
            tfree(config->vertices, config->vertex_size * config->vertex_count, MEMORY_TAG_ARRAY);
        }
        
        if(config->indices){
            tfree(config->indices, config->index_size * config->index_count, MEMORY_TAG_ARRAY);
        }

        tzero_memory(config, sizeof(geometry_config));
    }
}

void geometry_system_release(geometry* geometry){
    if(geometry && geometry->id != INVALID_ID){
        geometry_reference* ref = &state_ptr->registered_geometries[geometry->id];

        // Take copy of the id
        u32 id = geometry->id;
        if(ref->geometry.id == id){
            if(ref->reference_count > 0){
                ref->reference_count--;
            }

            // Also blanks out the geometry id.
            if(ref->reference_count < 1 && ref->auto_release){
                destroy_geometry(state_ptr, &ref->geometry);
                ref->reference_count = 0;
                ref->auto_release = FALSE;
            }
        } else{
            TFATAL("Geometry id mismatch. Check registration logic, as this should never occur.");
        }

        return;
    }

    TWARN("geometry_system_release cannot release invalid geometry id. Nothing was done.");
}

geometry* geometry_system_get_default(){
    if(state_ptr){
        return &state_ptr->default_geometry;
    }

    TFATAL("geometry_system_get_default called before system was initialized. Returning nullptr.");
    return 0;
}

geometry* geometry_system_get_default_2d(){
    if(state_ptr){
        return &state_ptr->default_2d_geometry;
    }

    TFATAL("geometry_system_get_edfault_2d called before system was initialized. Returning nullptr.");
    return 0;
}

b8 create_geometry(geometry_system_state* state, geometry_config config, geometry* g){
    // Send the geometry off the renderer to be uploaded to the GPU.
    if(!renderer_create_geometry(g, config.vertex_size, config.vertex_count, config.vertices, config.index_size, config.index_count, config.indices)){
        // Invalidate the entry.
        state->registered_geometries[g->id].reference_count = 0;
        state->registered_geometries[g->id].auto_release = FALSE;
        g->id = INVALID_ID;
        g->generation = INVALID_ID;
        g->internal_id = INVALID_ID;

        return FALSE;
    }

    // Acquire the material
    if(string_length(config.material_name) > 0){
        g->material = material_system_acquire(config.material_name);
        if(!g->material){
            g->material = material_system_get_default();
        }
    }

    return TRUE;
}

void destroy_geometry(geometry_system_state* state, geometry* g){
    renderer_destroy_geometry(g);
    g->internal_id = INVALID_ID;
    g->generation = INVALID_ID;
    g->id = INVALID_ID;

    string_empty(g->name);

    // Release the material.
    if(g->material && string_length(g->material->name) > 0){
        material_system_release(g->material->name);
        g->material = 0;
    }
}

b8 create_default_geometries(geometry_system_state* state){
    vertex_3d verts[4];
    tzero_memory(verts, sizeof(vertex_3d) * 4);

    const f32 f = 10.0f;

    verts[0].position.x = -0.5 * f;
    verts[0].position.y = -0.5 * f;
    verts[0].texcoord.x = 0.0f;
    verts[0].texcoord.y = 0.0f;


    verts[1].position.x = 0.5 * f;
    verts[1].position.y = 0.5 * f;
    verts[1].texcoord.x = 1.0f;
    verts[1].texcoord.y = 1.0f;

    verts[2].position.x = -0.5 * f;
    verts[2].position.y = 0.5 * f;
    verts[2].texcoord.x = 0.0f;
    verts[2].texcoord.y = 1.0f;
    
    verts[3].position.x = 0.5 * f;
    verts[3].position.y = -0.5 * f;
    verts[3].texcoord.x = 1.0f;
    verts[3].texcoord.y = 0.0f;

    u32 indices[6] = {0, 1, 2, 0, 3, 1};

    // Send the geometry off to the renderer to be uploaded to the GPU.
    state->default_geometry.internal_id = INVALID_ID;
    if(!renderer_create_geometry(&state->default_geometry, sizeof(vertex_3d),4, verts, sizeof(u32), 6, indices)){
        TFATAL("Failed to create default geometry. Application cannot continue.");
        return FALSE;
    }

    // Acquire the default material.
    state->default_geometry.material = material_system_get_default();

    // Create default 2d geometry.
    vertex_2d verts2d[4];
    tzero_memory(verts2d, sizeof(vertex_3d) * 4);
    verts2d[0].position.x = -0.5 * f;
    verts2d[0].position.y = -0.5 * f;
    verts2d[0].texcoord.x = 0.0f;
    verts2d[0].texcoord.y = 0.0f;


    verts2d[1].position.x = 0.5 * f;
    verts2d[1].position.y = 0.5 * f;
    verts2d[1].texcoord.x = 1.0f;
    verts2d[1].texcoord.y = 1.0f;

    verts2d[2].position.x = -0.5 * f;
    verts2d[2].position.y = 0.5 * f;
    verts2d[2].texcoord.x = 0.0f;
    verts2d[2].texcoord.y = 1.0f;
    
    verts2d[3].position.x = 0.5 * f;
    verts2d[3].position.y = -0.5 * f;
    verts2d[3].texcoord.x = 1.0f;
    verts2d[3].texcoord.y = 0.0f;

    // Indices (NOTE: counter-clockwise)
    u32 indices2d[6] = {2, 1, 0, 3, 0, 1};

    // Send the geometry off to the renderer to be uploaded to the GPU.
    if(!renderer_create_geometry(&state->default_2d_geometry, sizeof(vertex_2d),4, verts2d, sizeof(u32), 6, indices2d)){
        TFATAL("Failed to create default 2d geometry. Application cannot continue.");
        return FALSE;
    }

    // Acquire the default material.
    state->default_2d_geometry.material = material_system_get_default();

    return TRUE;
}

geometry_config geometry_system_generate_plane_config(f32 width, f32 height, u32 x_segment_count, u32 y_segment_count, f32 tile_x, f32 tile_y, const char* name, const char* material_name){
    if(width == 0){
        TWARN("Width must be nonzero. Defaulting to one.");
        width = 1.0f;
    }

    if(height == 0){
        TWARN("Height must be nonzero. Defaulting to one.");
        height = 1.0f;
    }

    if(x_segment_count < 1){
        TWARN("x_segment_count must be a positive number. Defaulting to one.");
        x_segment_count = 1;
    }
    
    if(y_segment_count < 1){
        TWARN("y_segment_count must be a positive number. Defaulting to one.");
        y_segment_count = 1;
    }
    
    if(tile_x == 0){
        TWARN("tile_x must be nonzero. Defaulting to one.");
        tile_x = 1.0f;
    }
    
    if(tile_y == 0){
        TWARN("tile_y must be nonzero. Defaulting to one.");
        tile_y = 1.0f;
    }

    geometry_config config;
    config.vertex_size = sizeof(vertex_3d);
    config.vertex_count = x_segment_count * y_segment_count * 4; // 4 verts per segment
    config.vertices = tallocate(sizeof(vertex_3d) * config.vertex_count, MEMORY_TAG_ARRAY);
    config.index_size = sizeof(u32);
    config.index_count = x_segment_count * y_segment_count * 6; // 6 indices per segment
    config.indices = tallocate(sizeof(u32) * config.index_count, MEMORY_TAG_ARRAY);

    // TODO: This generates extra vertices, but we can always deduplicate them later.
    f32 seg_width = width / x_segment_count;
    f32 seg_height = height / y_segment_count;
    f32 half_width = width * 0.5f;
    f32 half_height = height * 0.5f;
    for(u32 y = 0; y < y_segment_count; ++y){
        for(u32 x = 0; x < x_segment_count; ++x){
            // Generate vertices
            f32 min_x = (x * seg_width) - half_width;
            f32 min_y = (y * seg_height) - half_height;
            f32 max_x = min_x + seg_width;
            f32 max_y = min_y + seg_height;
            f32 min_uvx = (x / (f32)x_segment_count) * tile_x;
            f32 min_uvy = (y / (f32)y_segment_count) * tile_y;
            f32 max_uvx = ((x + 1) / (f32)x_segment_count) * tile_x;
            f32 max_uvy = ((y + 1) / (f32)y_segment_count) * tile_y;

            u32 v_offset = ((y * x_segment_count) + x) * 4;
            vertex_3d* v0 = &((vertex_3d*)config.vertices)[v_offset + 0];
            vertex_3d* v1 = &((vertex_3d*)config.vertices)[v_offset + 1];
            vertex_3d* v2 = &((vertex_3d*)config.vertices)[v_offset + 2];
            vertex_3d* v3 = &((vertex_3d*)config.vertices)[v_offset + 3];

            v0->position.x = min_x;
            v0->position.y = min_y;
            v0->texcoord.x = min_uvx;
            v0->texcoord.y = min_uvy;
        
            v1->position.x = max_x;
            v1->position.y = max_y;
            v1->texcoord.x = max_uvx;
            v1->texcoord.y = max_uvy;
            
            v2->position.x = min_x;
            v2->position.y = max_y;
            v2->texcoord.x = min_uvx;
            v2->texcoord.y = max_uvy;
            
            v3->position.x = max_x;
            v3->position.y = min_y;
            v3->texcoord.x = max_uvx;
            v3->texcoord.y = min_uvy;
        
            // Generate indices
            u32 i_offset = ((y * x_segment_count) + x) * 6;
            ((u32*)config.indices)[i_offset + 0] = v_offset + 0;
            ((u32*)config.indices)[i_offset + 1] = v_offset + 1;
            ((u32*)config.indices)[i_offset + 2] = v_offset + 2;
            ((u32*)config.indices)[i_offset + 3] = v_offset + 0;
            ((u32*)config.indices)[i_offset + 4] = v_offset + 3;
            ((u32*)config.indices)[i_offset + 5] = v_offset + 1;
        }
    }

    if(name && string_length(name) > 0){
        string_ncopy(config.name, name, GEOMETRY_NAME_MAX_LENGTH);
    } else {
        string_ncopy(config.name, DEFAULT_GEOMETRY_NAME, GEOMETRY_NAME_MAX_LENGTH);
    }

    if(material_name && string_length(material_name) > 0){
        string_ncopy(config.material_name, material_name, MATERIAL_NAME_MAX_LENGTH);
    } else {
        string_ncopy(config.material_name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    }

    return config;
}



geometry_config geometry_system_generate_cube_config(f32 width, f32 height, f32 depth, f32 tile_x, f32 tile_y, const char* name, const char* material_name) {
    if (width == 0) {
        TWARN("Width must be nonzero. Defaulting to one.");
        width = 1.0f;
    }

    if (height == 0) {
        TWARN("Height must be nonzero. Defaulting to one.");
        height = 1.0f;
    }

    if (depth == 0) {
        TWARN("Depth must be nonzero. Defaulting to one.");
        depth = 1;
    }

    if (tile_x == 0) {
        TWARN("tile_x must be nonzero. Defaulting to one.");
        tile_x = 1.0f;
    }

    if (tile_y == 0) {
        TWARN("tile_y must be nonzero. Defaulting to one.");
        tile_y = 1.0f;
    }



    geometry_config config;
    config.vertex_size = sizeof(vertex_3d);
    config.vertex_count = 4 * 6;  // 4 verts per side, 6 sides
    config.vertices = tallocate(sizeof(vertex_3d) * config.vertex_count, MEMORY_TAG_ARRAY);
    config.index_size = sizeof(u32);
    config.index_count = 6 * 6;  // 6 indices per side, 6 sides
    config.indices = tallocate(sizeof(u32) * config.index_count, MEMORY_TAG_ARRAY);

    f32 half_width = width * 0.5f;
    f32 half_height = height * 0.5f;
    f32 half_depth = depth * 0.5f;
    f32 min_x = -half_width;
    f32 min_y = -half_height;
    f32 min_z = -half_depth;
    f32 max_x = half_width;
    f32 max_y = half_height;
    f32 max_z = half_depth;
    f32 min_uvx = 0.0f;
    f32 min_uvy = 0.0f;
    f32 max_uvx = tile_x;
    f32 max_uvy = tile_y;

    vertex_3d verts[24];
    // Front face
    verts[(0 * 4) + 0].position = (vec3){min_x, min_y, max_z};
    verts[(0 * 4) + 1].position = (vec3){max_x, max_y, max_z};
    verts[(0 * 4) + 2].position = (vec3){min_x, max_y, max_z};
    verts[(0 * 4) + 3].position = (vec3){max_x, min_y, max_z};
    verts[(0 * 4) + 0].texcoord = (vec2){min_uvx, min_uvy};
    verts[(0 * 4) + 1].texcoord = (vec2){max_uvx, max_uvy};
    verts[(0 * 4) + 2].texcoord = (vec2){min_uvx, max_uvy};
    verts[(0 * 4) + 3].texcoord = (vec2){max_uvx, min_uvy};
    verts[(0 * 4) + 0].normal = (vec3){0, 0, 1};
    verts[(0 * 4) + 1].normal = (vec3){0, 0, 1};
    verts[(0 * 4) + 2].normal = (vec3){0, 0, 1};
    verts[(0 * 4) + 3].normal = (vec3){0, 0, 1};

    // Back face
    verts[(1 * 4) + 0].position = (vec3){max_x, min_y, min_z};
    verts[(1 * 4) + 1].position = (vec3){min_x, max_y, min_z};
    verts[(1 * 4) + 2].position = (vec3){max_x, max_y, min_z};
    verts[(1 * 4) + 3].position = (vec3){min_x, min_y, min_z};
    verts[(1 * 4) + 0].texcoord = (vec2){min_uvx, min_uvy};
    verts[(1 * 4) + 1].texcoord = (vec2){max_uvx, max_uvy};
    verts[(1 * 4) + 2].texcoord = (vec2){min_uvx, max_uvy};
    verts[(1 * 4) + 3].texcoord = (vec2){max_uvx, min_uvy};
    verts[(1 * 4) + 0].normal = (vec3){0, 0, -1};
    verts[(1 * 4) + 1].normal = (vec3){0, 0, -1};
    verts[(1 * 4) + 2].normal = (vec3){0, 0, -1};
    verts[(1 * 4) + 3].normal = (vec3){0, 0, -1};

    // Left
    verts[(2 * 4) + 0].position = (vec3){min_x, min_y, min_z};
    verts[(2 * 4) + 1].position = (vec3){min_x, max_y, max_z};
    verts[(2 * 4) + 2].position = (vec3){min_x, max_y, min_z};
    verts[(2 * 4) + 3].position = (vec3){min_x, min_y, max_z};
    verts[(2 * 4) + 0].texcoord = (vec2){min_uvx, min_uvy};
    verts[(2 * 4) + 1].texcoord = (vec2){max_uvx, max_uvy};
    verts[(2 * 4) + 2].texcoord = (vec2){min_uvx, max_uvy};
    verts[(2 * 4) + 3].texcoord = (vec2){max_uvx, min_uvy};
    verts[(2 * 4) + 0].normal = (vec3){-1, 0, 0};
    verts[(2 * 4) + 1].normal = (vec3){-1, 0, 0};
    verts[(2 * 4) + 2].normal = (vec3){-1, 0, 0};
    verts[(2 * 4) + 3].normal = (vec3){-1, 0, 0};

    // Right face
    verts[(3 * 4) + 0].position = (vec3){max_x, min_y, max_z};
    verts[(3 * 4) + 1].position = (vec3){max_x, max_y, min_z};
    verts[(3 * 4) + 2].position = (vec3){max_x, max_y, max_z};
    verts[(3 * 4) + 3].position = (vec3){max_x, min_y, min_z};
    verts[(3 * 4) + 0].texcoord = (vec2){min_uvx, min_uvy};
    verts[(3 * 4) + 1].texcoord = (vec2){max_uvx, max_uvy};
    verts[(3 * 4) + 2].texcoord = (vec2){min_uvx, max_uvy};
    verts[(3 * 4) + 3].texcoord = (vec2){max_uvx, min_uvy};
    verts[(3 * 4) + 0].normal = (vec3){1, 0, 0};
    verts[(3 * 4) + 1].normal = (vec3){1, 0, 0};
    verts[(3 * 4) + 2].normal = (vec3){1, 0, 0};
    verts[(3 * 4) + 3].normal = (vec3){1, 0, 0};

    // Bottom face
    verts[(4 * 4) + 0].position = (vec3){max_x, min_y, max_z};
    verts[(4 * 4) + 1].position = (vec3){min_x, min_y, min_z};
    verts[(4 * 4) + 2].position = (vec3){max_x, min_y, min_z};
    verts[(4 * 4) + 3].position = (vec3){min_x, min_y, max_z};
    verts[(4 * 4) + 0].texcoord = (vec2){min_uvx, min_uvy};
    verts[(4 * 4) + 1].texcoord = (vec2){max_uvx, max_uvy};
    verts[(4 * 4) + 2].texcoord = (vec2){min_uvx, max_uvy};
    verts[(4 * 4) + 3].texcoord = (vec2){max_uvx, min_uvy};
    verts[(4 * 4) + 0].normal = (vec3){0, -1, 0};
    verts[(4 * 4) + 1].normal = (vec3){0, -1, 0};
    verts[(4 * 4) + 2].normal = (vec3){0, -1, 0};
    verts[(4 * 4) + 3].normal = (vec3){0, -1, 0};

    // Top face
    verts[(5 * 4) + 0].position = (vec3){min_x, max_y, max_z};
    verts[(5 * 4) + 1].position = (vec3){max_x, max_y, min_z};
    verts[(5 * 4) + 2].position = (vec3){min_x, max_y, min_z};
    verts[(5 * 4) + 3].position = (vec3){max_x, max_y, max_z};
    verts[(5 * 4) + 0].texcoord = (vec2){min_uvx, min_uvy};
    verts[(5 * 4) + 1].texcoord = (vec2){max_uvx, max_uvy};
    verts[(5 * 4) + 2].texcoord = (vec2){min_uvx, max_uvy};
    verts[(5 * 4) + 3].texcoord = (vec2){max_uvx, min_uvy};
    verts[(5 * 4) + 0].normal = (vec3){0, 1, 0};
    verts[(5 * 4) + 1].normal = (vec3){0, 1, 0};
    verts[(5 * 4) + 2].normal = (vec3){0, 1, 0};
    verts[(5 * 4) + 3].normal = (vec3){0, 1, 0};

    tcopy_memory(config.vertices, verts, config.vertex_size * config.vertex_count);

    for (u32 i = 0; i < 6; ++i) {
        u32 v_offset = i * 4;
        u32 i_offset = i * 6;
        ((u32*)config.indices)[i_offset + 0] = v_offset + 0;
        ((u32*)config.indices)[i_offset + 1] = v_offset + 1;
        ((u32*)config.indices)[i_offset + 2] = v_offset + 2;
        ((u32*)config.indices)[i_offset + 3] = v_offset + 0;
        ((u32*)config.indices)[i_offset + 4] = v_offset + 3;
        ((u32*)config.indices)[i_offset + 5] = v_offset + 1;
    }

    if (name && string_length(name) > 0) {
        string_ncopy(config.name, name, GEOMETRY_NAME_MAX_LENGTH);
    } else {
        string_ncopy(config.name, DEFAULT_GEOMETRY_NAME, GEOMETRY_NAME_MAX_LENGTH);
    }

    if (material_name && string_length(material_name) > 0) {
        string_ncopy(config.material_name, material_name, MATERIAL_NAME_MAX_LENGTH);
    } else {
        string_ncopy(config.material_name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LENGTH);
    }

    geometry_generate_tangents(config.vertex_count, config.vertices, config.index_count, config.indices);

    return config;
}