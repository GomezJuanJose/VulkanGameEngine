#pragma once
#include "defines.h"

#include "resources/resource_types.h"

#define DEFAULT_MATERIAL_NAME "default"

typedef struct material_system_config{
    u32 max_material_count;
} material_system_config;



b8 material_system_initialize(u64* memory_requirement, void* state, material_system_config config);
void material_system_shutdown(void* state);

material* material_system_acquire(const char* name);
material* material_system_acquire_from_config(material_config config);
void material_system_release(const char* name);

material* material_system_get_default();

/**
 * @brief Applies global-level data for the material shader id.
 * 
 * @param shader_id The identifier of the shader to apply globals for.
 * @param projection A constant pointer to a projection matrix.
 * @param view A constant pointer to a view matrix.
 * @param ambient_colour The ambient colour of the scene.
 * @param view_position The camera position.
 * @return True on success; otherwise false.
 */
b8 material_system_apply_global(u32 shader_id, const mat4* projection, const mat4* view, const vec4* ambient_colour, const vec3* view_position);

/**
 * @brief Applies instance-level material data for the given material.
 * 
 * @param m A pointer to the material to be applied.
 * @return True on success; otherwise false.
 */
b8 material_system_apply_instance(material* m);

/**
 * @brief Applies local-level material data (typically just model matrix).
 * 
 * @param m A pointer to the material to be applied.
 * @param model A constant pointer to the model matrix to be applied.
 * @return True on success; otherwise false.
 */
b8 material_system_apply_local(material* m, const mat4* model);