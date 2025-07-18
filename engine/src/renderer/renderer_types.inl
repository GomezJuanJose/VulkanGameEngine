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

typedef enum renderer_debug_view_mode{
    RENDERER_VIEW_MODE_DEFAULT = 0,
    RENDERER_VIEW_MODE_LIGHTING = 1,
    RENDERER_VIEW_MODE_NORMALS = 2
} renderer_debug_view_mode;

/** @brief Represents a render target, which is used for rendering to a texture or set of textures. */
typedef struct render_target {
    /** @brief Indicates if this render target should be updated on window resize. */
    b8 sync_to_window_size;
    /** @brief The number of attachments */
    u8 attachment_count;
    /** @brief An array of attachments (pointers to textures). */
    struct texture** attachments;
    /** @brief The renderer API internal framebuffer object. */
    void* internal_framebuffer;
} render_target;

/**
 * @brief The types of clearing to be done on a renderpass.
 * Can be combined together for multiple clearing functions.
 */
typedef enum renderpass_clear_flag {
    /** @brief No clearing shoudl be done. */
    RENDERPASS_CLEAR_NONE_FLAG = 0x0,
    /** @brief Clear the colour buffer. */
    RENDERPASS_CLEAR_COLOUR_BUFFER_FLAG = 0x1,
    /** @brief Clear the depth buffer. */
    RENDERPASS_CLEAR_DEPTH_BUFFER_FLAG = 0x2,
    /** @brief Clear the stencil buffer. */
    RENDERPASS_CLEAR_STENCIL_BUFFER_FLAG = 0x4
} renderpass_clear_flag;

typedef struct renderpass_config {
    /** @brief The name of this renderpass. */
    const char* name;
    /** @brief The name of the previous renderpass. */
    const char* prev_name;
    /** @brief The name of the next renderpass. */
    const char* next_name;
    /** @brief The current render area of the renderpass. */
    vec4 render_area;
    /** @brief The clear colour used for this renderpass. */
    vec4 clear_colour;

    /** @brief The clear flags for this renderpass. */
    u8 clear_flags;
} renderpass_config;

/**
 * @brief Represents a generic renderpass.
 */
typedef struct renderpass {
    /** @brief The id of the renderpass */
    u16 id;

    /** @brief The current render area of the renderpass. */
    vec4 render_area;
    /** @brief The clear colour used for this renderpass. */
    vec4 clear_colour;

    /** @brief The clear flags for this renderpass. */
    u8 clear_flags;
    /** @brief The number of render targets for this renderpass. */
    u8 render_target_count;
    /** @brief An array of render targets used by this renderpass. */
    render_target* targets;

    /** @brief Internal renderpass data */
    void* internal_data;
} renderpass;

/** @brief The generic configuration for a renderer backend. */
typedef struct renderer_backend_config {
    /** @brief The name of the application */
    const char* application_name;
    /** @brief The number of pointers to renderpasses. */
    u16 renderpass_count;
    /** @brief An array configurations for renderpasses. Will be initialized on the backend automatically. */
    renderpass_config* pass_configs;
    /** @brief A callback that will be made when the backend requires a refresh/regeneration of the render targets. */
    void (*on_rendertarget_refresh_required)();
} renderer_backend_config;

typedef struct renderer_backend {
    u64 frame_number;


    b8 (*initialize)(struct renderer_backend* backend, const renderer_backend_config* config, u8* out_window_render_target_count);

    void (*shutdown)(struct renderer_backend* backend);

    void (*resized)(struct renderer_backend* backend, u16 width, u16 height);

    b8 (*begin_frame)(struct renderer_backend* backend, f32 delta_time);
    void (*update_global_world_state)(mat4 projection, mat4 view, vec3 view_position, vec4 ambient_colour, i32 mode);
    void (*update_global_ui_state)(mat4 projection, mat4 view, i32 mode);
    b8 (*end_frame)(struct renderer_backend* backend, f32 delta_time);

    b8 (*begin_renderpass)(struct renderer_backend* backend, renderpass* pass, render_target* target);
    b8 (*end_renderpass)(struct renderer_backend* backend, renderpass* pass);
    renderpass* (*renderpass_get)(const char* name);

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
    b8 (*shader_create)(struct shader* shader, renderpass* pass, u8 stage_count, const char** stage_filenames, shader_stage* stages);



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

    /**
     * @brief Creates a new render target using the provided data.
     *
     * @param attachment_count The number of attachments (texture pointers).
     * @param attachments An array of attachments (texture pointers).
     * @param renderpass A pointer to the renderpass the render target is associated with.
     * @param width The width of the render target in pixels.
     * @param height The height of the render target in pixels.
     * @param out_target A pointer to hold the newly created render target.
     */
    void (*render_target_create)(u8 attachment_count, texture** attachments, renderpass* pass, u32 width, u32 height, render_target* out_target);

    /**
     * @brief Destroys the provided render target.
     *
     * @param target A pointer to the render target to be destroyed.
     * @param free_internal_memory Indicates if internal memory should be freed.
     */
    void (*render_target_destroy)(render_target* target, b8 free_internal_memory);

    /**
     * @brief Creates a new renderpass.
     *
     * @param out_renderpass A pointer to the generic renderpass.
     * @param depth The depth clear amount.
     * @param stencil The stencil clear value.
     * @param clear_flags The combined clear flags indicating what kind of clear should take place.
     * @param has_prev_pass Indicates if there is a previous renderpass.
     * @param has_next_pass Indicates if there is a next renderpass.
     */
    void (*renderpass_create)(renderpass* out_renderpass, f32 depth, u32 stencil, b8 has_prev_pass, b8 has_next_pass);

    /**
     * @brief Destroys the given renderpass.
     *
     * @param pass A pointer to the renderpass to be destroyed.
     */
    void (*renderpass_destroy)(renderpass* pass);

    /**
     * @brief Attempts to get the window render target at the given index.
     *
     * @param index The index of the attachment to get. Must be within the range of window render target count.
     * @return A pointer to a texture attachment if successful; otherwise 0.
     */
    texture* (*window_attachment_get)(u8 index);

    /**
     * @brief Returns a pointer to the main depth texture target.
     */
    texture* (*depth_attachment_get)();

    /**
     * @brief Returns the current window attachment index.
     */
    u8 (*window_attachment_index_get)();
} renderer_backend;

typedef struct render_packet{
    f32 delta_time;

    u32 geometry_count;
    geometry_render_data* geometries;

    u32 ui_geometry_count;
    geometry_render_data* ui_geometries;
} render_packet;