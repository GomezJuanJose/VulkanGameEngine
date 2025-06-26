#pragma once

#include "renderer_types.inl"

struct static_mesh_data;

struct shader;
struct shader_uniform;

b8 renderer_system_initialize(u64* memory_requirement, void* state, const char* application_name);
void renderer_system_shutdown(void* state);

void renderer_on_resized(u16 width, u16 height);

b8 renderer_draw_frame(render_packet* packet);

// HACK: this should not be expose outside the engine.
TAPI void renderer_set_view(mat4 view, vec3 view_position);

void renderer_create_texture(const u8* pixels, struct texture* texture);
void renderer_destroy_texture(struct texture* texture);

b8 renderer_create_material(struct material* material);
void renderer_destroy_material(struct material* material);

b8 renderer_create_geometry(geometry* geometry, u32 vertex_size, u32 vertex_count, const void* vertices, u32 index_size, u32 index_count, const void* indices);
void renderer_destroy_geometry(geometry* geometry);

/**
 * @brief Obtains the identifier of the renderpass with the given name.
 *
 * @param name The name of the renderpass whose identifier to obtain.
 * @param out_renderpass_id A pointer to hold the renderpass id.
 * @return True if found; otherwise false.
 */
b8 renderer_renderpass_id(const char* name, u8* out_renderpass_id);



/**
 * @brief Creates internal shader resources using the provided parameters.
 * 
 * @param s A pointer to the shader.
 * @param renderpass_id The identifier of the renderpass to be associated with the shader.
 * @param stage_count The total number of stages.
 * @param stage_filenames An array of shader stage filenames to be loaded. Should align with stages array.
 * @param stages A array of shader_stages indicating what render stages (vertex, fragment, etc.) used in this shader.
 * @return b8 True on success; otherwise false.
 */
b8 renderer_shader_create(struct shader* s, u8 renderpass_id, u8 stage_count, const char** stage_filenames, shader_stage* stages);



/**
 * @brief Destroys the given shader and releases any resources held by it.
 * @param s A pointer to the shader to be destroyed.
 */
void renderer_shader_destroy(struct shader* s);



/**
 * @brief Initializes a configured shader. Will be automatically destroyed if this step fails.
 * Must be done after vulkan_shader_create().
 *
 * @param s A pointer to the shader to be initialized.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_initialize(struct shader* s);



/**
 * @brief Uses the given shader, activating it for updates to attributes, uniforms and such,
 * and for use in draw calls.
 *
 * @param s A pointer to the shader to be used.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_use(struct shader* s);



/**
 * @brief Binds global resources for use and updating.
 *
 * @param s A pointer to the shader whose globals are to be bound.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_bind_globals(struct shader* s);



/**
 * @brief Binds instance resources for use and updating.
 *
 * @param s A pointer to the shader whose instance resources are to be bound.
 * @param instance_id The identifier of the instance to be bound.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_bind_instance(struct shader* s, u32 instance_id);



/**
 * @brief Applies global data to the uniform buffer.
 *
 * @param s A pointer to the shader to apply the global data for.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_apply_globals(struct shader* s);



/**
 * @brief Applies data for the currently bound instance.
 *
 * @param s A pointer to the shader to apply the instance data for.
 * @param needs_update Indicates if the shader uniforms need to be updated or just bound.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_apply_instance(struct shader* s, b8 needs_update);



/**
 * @brief Acquires internal instance-level resources and provides an instance id.
 *
 * @param s A pointer to the shader to acquire resources from.
 * @param out_instance_id A pointer to hold the new instance identifier.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_acquire_instance_resources(struct shader* s, u32* out_instance_id);



/**
 * @brief Releases internal instance-level resources for the given instance id.
 *
 * @param s A pointer to the shader to release resources from.
 * @param instance_id The instance identifier whose resources are to be released.
 * @return True on success; otherwise false.
 */
b8 renderer_shader_release_instance_resources(struct shader* s, u32 instance_id);



/**
 * @brief Sets the uniform of the given shader to the provided value.
 * 
 * @param s A ponter to the shader.
 * @param uniform A constant pointer to the uniform.
 * @param value A pointer to the value to be set.
 * @return b8 True on success; otherwise false.
 */
b8 renderer_set_uniform(struct shader* s, struct shader_uniform* uniform, const void* value);