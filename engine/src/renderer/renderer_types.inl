#pragma once

#include "defines.h"
#include "math/math_types.h"
#include "resources/resource_types.h"

#define BUILTIN_SHADER_NAME_MATERIAL "Shader.Builtin.Material"
#define BUILTIN_SHADER_NAME_UI "Shader.Builtin.UI"

struct shader;
struct shader_uniform;

typedef enum renderer_backend_type{
    RENDERER_BACKEND_TYPE_VULKAN,
    RENDERER_BACKEND_TYPE_OPENGL,
    RENDERER_BACKEND_TYPE_DIRECTX
} renderer_backend_type;

typedef struct geometry_render_data{
    mat4 model;
    geometry* geometry;
} geometry_render_data;

typedef enum builtin_renderpass{
    BUILTIN_RENDERPASS_WORLD = 0x01,
    BUILTIN_RENDERPASS_UI = 0x02
} builtin_renderpass;

typedef enum renderer_debug_view_mode{
    RENDERER_VIEW_MODE_DEFAULT = 0,
    RENDERER_VIEW_MODE_LIGHTING = 1,
    RENDERER_VIEW_MODE_NORMALS = 2
} renderer_debug_view_mode;

typedef struct renderer_backend {
    u64 frame_number;


    b8 (*initialize)(struct renderer_backend* backend, const char* application_name);

    void (*shutdown)(struct renderer_backend* backend);

    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_global_world_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
    void (*update_global_ui_state)(mat4 projection, mat4 view, i32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);

    b8 (*begin_renderpass)(struct renderer_backend* backend, u8 renderpass_id);
    b8 (*end_renderpass)(struct renderer_backend* backend, u8 renderpass_id);

    void (*draw_geometry)(geometry_render_data data);

    void (*texture_create)(const u8* pixels, struct texture* texture);
    void (*texture_destroy)(struct texture* texture);
    /**
     * @brief Creates a new writeable texture with no data written to it.
     *
     * @param t A pointer to the texture to hold the resources.
     */
    void (*texture_create_writeable)(texture* t);

    /**
     * @brief Resizes a texture. There is no check at this level to see if the
     * texture is writeable. Internal resources are destroyed and re-created at
     * the new resolution. Data is lost and would need to be reloaded.
     *
     * @param t A pointer to the texture to be resized.
     * @param new_width The new width in pixels.
     * @param new_height The new height in pixels.
     */
    void (*texture_resize)(texture* t, u32 new_width, u32 new_height);

    /**
     * @brief Writes the given data to the provided texture.
     * NOTE: At this level, this can either be a writeable or non-writeable texture because
     * this also handles the initial texture load. The texture system itself should be
     * responsible for blocking write requests to non-writeable textures.
     *
     * @param t A pointer to the texture to be written to.
     * @param offset The offset in bytes from the beginning of the data to be written.
     * @param size The number of bytes to be written.
     * @param pixels The raw image data to be written.
     */
    void (*texture_write_data)(texture* t, u32 offset, u32 size, const u8* pixels);

    b8 (*create_material)(struct material* material);
    void (*destroy_material)(struct material* material);

    b8 (*create_geometry)(geometry* geometry, u32 vertex_size, u32 vertex_count, const void* vertices, u32 index_size, u32 index_count, const void* indices);
    void (*destroy_geometry)(geometry* geometry);

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
    b8 (*shader_create)(struct shader* shader, u8 renderpass_id, u8 stage_count, const char** stage_filenames, shader_stage* stages);



    /**
     * @brief Destroys the given shader and releases any resources held by it.
     * @param s A pointer to the shader to be destroyed.
     */
    void (*shader_destroy)(struct shader* shader);



    /**
     * @brief Initializes a configured shader. Will be automatically destroyed if this step fails.
     * Must be done after vulkan_shader_create().
     *
     * @param s A pointer to the shader to be initialized.
     * @return True on success; otherwise false.
     */
    b8 (*shader_initialize)(struct shader* shader);



    /**
     * @brief Uses the given shader, activating it for updates to attributes, uniforms and such,
     * and for use in draw calls.
     *
     * @param s A pointer to the shader to be used.
     * @return True on success; otherwise false.
     */
    b8 (*shader_use)(struct shader* shader);



    /**
     * @brief Binds global resources for use and updating.
     *
     * @param s A pointer to the shader whose globals are to be bound.
     * @return True on success; otherwise false.
     */
    b8 (*shader_bind_globals)(struct shader* s);



    /**
     * @brief Binds instance resources for use and updating.
     *
     * @param s A pointer to the shader whose instance resources are to be bound.
     * @param instance_id The identifier of the instance to be bound.
     * @return True on success; otherwise false.
     */
    b8 (*shader_bind_instance)(struct shader* s, u32 instance_id);



    /**
     * @brief Applies global data to the uniform buffer.
     *
     * @param s A pointer to the shader to apply the global data for.
     * @return True on success; otherwise false.
     */
    b8 (*shader_apply_globals)(struct shader* s);

    /**
     * @brief Applies data  for the currently bound instance.
     * 
     * @param s A pointer to the shader to apply the instance data for.
     * @return True on success; otherwise false.
     */
    b8 (*shader_apply_instance)(struct shader* s, b8 needs_update);

    /**
     * @brief Acquires internal instance-level resources and provides an instace id.
     * 
     * @param s A pointer to the shader to acquire resources from.
     * @param maps An array of pointers to texture maps. Must be one map per instance texture.
     * @param out_instance_id A pointer to hold the new instance identifier.
     * @return True on success; otherwise false.
     */
    b8 (*shader_acquire_instance_resources)(struct shader* s, texture_map** maps, u32* out_instance_id);

    /**
     * @brief Releases internal instance-level resources for the given instance id.
     * 
     * @param s A pointer to the shader to release resources from.
     * @param instance_id The instance identifier whose resources are to be released.
     * @return True on success; otherwise false.
     */
    b8 (*shader_release_instance_resources)(struct shader* s, u32 instance_id);


    /**
     * @brief Sets the uniform of the given shader to the provided value.
     * 
     * @param s A pointer to the shader.
     * @param uniform A constant point to the uniform.
     * @param value A pointer to the value to be set.
     * @return b8 True on successM otherwise false.
     */
    b8 (*shader_set_uniform)(struct shader* frontend_shader, struct shader_uniform* uniform, const void* value);

    /**
     * @brief Acquires internal resources for the given texture map.Add commentMore actions
     * 
     * @param map A pointer to the texture map to obtain resources for.
     * @return True on success; otherwise false.
     */
    b8 (*texture_map_acquire_resources)(struct texture_map* map);

    /**
     * @brief Releases internal resources for the given texture map.
     * 
     * @param map A pointer to the texture map to release resources from.
     */
    void (*texture_map_release_resources)(struct texture_map* map);
} renderer_backend;

typedef struct render_packet{
    f32 delta_time;

    u32 geometry_count;
    geometry_render_data* geometries;

    u32 ui_geometry_count;
    geometry_render_data* ui_geometries;
} render_packet;