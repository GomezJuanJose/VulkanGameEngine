#pragma once

#include "renderer/vulkan/vulkan_types.inl"
#include "renderer/renderer_types.inl"

b8 vulkan_material_shader_create(vulkan_context* context, vulkan_material_shader* out_shader);

void vulkan_material_shader_destroy(vulkan_context* context, struct vulkan_material_shader* shader);

void vulkan_material_shader_use(vulkan_context* context, struct vulkan_material_shader* shader);

void vulkan_material_shader_update_global_state(vulkan_context* context, struct vulkan_material_shader* shader, f32 delta_time);

void vulkan_material_shader_update_object(vulkan_context* context, struct vulkan_material_shader* shader, geometry_render_data data);

b8 vulkan_material_shader_acquire_resources(vulkan_context* context, struct vulkan_material_shader* shader, u32* out_object_id);
void vulkan_material_shader_release_resources(vulkan_context* context, struct vulkan_material_shader* shader, u32 object_id);