#pragma once

#include "defines.h"

#include "core/asserts.h"

#include "renderer/renderer_types.inl"
#include "containers/freelist.h"
#include "containers/hashtable.h"

#include <vulkan/vulkan.h>

// Checks the given expression's return value against VK_SUCCESS.
#define VK_CHECK(expr)                      \
        {                                   \
            TASSERT(expr == VK_SUCCESS);    \
        }

struct vulkan_context;

typedef struct vulkan_buffer{
    u64 total_size;
    VkBuffer handle;
    VkBufferUsageFlagBits usage;
    b8 is_locked;
    VkDeviceMemory memory;
    i32 memory_index;
    u32 memory_property_flags;
    /** @brief The amount of memory required for the freelist. */
    u64 freelist_memory_requirement;
    /** @brief The memory block used by the internal freelist. */
    void* freelist_block;
    /** @brief A freelist to track allocations. */
    freelist buffer_freelist;
    b8 has_freelist;
    
} vulkan_buffer;

typedef struct vulkan_swapchain_support_info{
    VkSurfaceCapabilitiesKHR capabilities;
    u32 format_count;
    VkSurfaceFormatKHR* formats;
    u32 present_mode_count;
    VkPresentModeKHR* present_modes;
} vulkan_swapchain_support_info;

typedef struct vulkan_device{
    VkPhysicalDevice physical_device;
    VkDevice logical_device;
    vulkan_swapchain_support_info swapchain_support;
    i32 graphics_queue_index;
    i32 present_queue_index;
    i32 transfer_queue_index;
    b8 supports_device_local_host_visible;

    VkQueue graphics_queue;
    VkQueue present_queue;
    VkQueue transfer_queue;

    VkCommandPool graphics_command_pool;

    VkPhysicalDeviceProperties properties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties memory;

    VkFormat depth_format;

    u8 depth_channel_count;
} vulkan_device;

typedef struct vulkan_image{
    VkImage handle;
    VkDeviceMemory memory;
    VkImageView view;
    u32 width;
    u32 height;
} vulkan_image;

typedef enum vulkan_render_pass_state{
    READY,
    RECORDING,
    IN_RENDER_PASS,
    RECORDING_ENDED,
    SUBMITTED,
    NOT_ALLOCATED
} vulkan_render_pass_state;

typedef struct vulkan_renderpass{
    VkRenderPass handle;

    f32 depth;
    u32 stencil;

    b8 has_prev_pass;
    b8 has_next_pass;

    vulkan_render_pass_state state;
} vulkan_renderpass;

typedef struct vulkan_swapchain{
    VkSurfaceFormatKHR image_format;
    u8 max_frames_in_flight;
    VkSwapchainKHR handle;
    u32 image_count;
    /** @brief An array of pointers to render targets, which contain swapchain images. */
    texture** render_textures;

    
    /** @brief The depth texture. */
    texture* depth_texture;

    /** 
     * @brief Render targets used for on-screen rendering, one per frame. 
     * The images contained in these are created and owned by the swapchain.
     **/
    render_target render_targets[3];
} vulkan_swapchain;

typedef enum vulkan_command_buffer_state{
    COMMAND_BUFFER_STATE_READY,
    COMMAND_BUFFER_STATE_RECORDING,
    COMMAND_BUFFER_STATE_IN_RENDER_PASS,
    COMMAND_BUFFER_STATE_RECORDING_ENDED,
    COMMAND_BUFFER_STATE_SUBMITTED,
    COMMAND_BUFFER_STATE_NOT_ALLOCATED
} vulkan_command_buffer_state;

typedef struct vulkan_command_buffer{
    VkCommandBuffer handle;

    // Command buffer state.
    vulkan_command_buffer_state state;
} vulkan_command_buffer;

typedef struct vulkan_shader_stage {
    VkShaderModuleCreateInfo create_info;
    VkShaderModule handle;
    VkPipelineShaderStageCreateInfo shader_stage_create_info;
} vulkan_shader_stage;

typedef struct vulkan_pipeline{
    VkPipeline handle;
    VkPipelineLayout pipeline_layout;
} vulkan_pipeline;


// Max number of material instances
// TODO: make configurable
#define VULKAN_MAX_MATERIAL_COUNT 1024

// Max number of simultaneously uploaded geometries
// TODO: make configurable
#define VULKAN_MAX_GEOMETRY_COUNT 4096

/**
 * @brief Internal buffer data for geometry.
 */
typedef struct vulkan_geometry_data{
    u32 id;
    u32 generation;
    u32 vertex_count;
    u32 vertex_element_size;
    u64 vertex_buffer_offset;
    u32 index_count;
    u32 index_element_size;
    u64 index_buffer_offset;
} vulkan_geometry_data;

#define VULKAN_SHADER_MAX_STAGES 8
#define VULKAN_SHADER_MAX_GLOBAL_TEXTURES 31
#define VULKAN_SHADER_MAX_INSTANCE_TEXTURES 31
#define VULKAN_SHADER_MAX_ATTRIBUTES 16

#define VULKAN_SHADER_MAX_UNIFORMS 128

#define VULKAN_SHADER_MAX_BINDINGS 2
#define VULKAN_SHADER_MAX_PUSH_CONST_RANGES 32




#define UI_SHADER_STAGE_COUNT 2
#define VULKAN_UI_SHADER_DESCRIPTOR_COUNT 2
#define VULKAN_UI_SHADER_SAMPLER_COUNT 1
// Max number of ui instances
// TODO: make configurable
#define VULKAN_MAX_UI_COUNT 1024

/**
 * @brief Configuration for a shader stage, such as vertex or fragment.
 */
typedef struct vulkan_shader_stage_config{

    /** @brief The shader stage bit flag */
    VkShaderStageFlagBits stage;
    /** @brief The shader file name. */
    char file_name[255];

} vulkan_shader_stage_config;

/**
 * @brief The configuration for a descriptor set.
 */
typedef struct vulkan_descriptor_set_config{
    /** @brief The number of bindings in this set. */
    u8 binding_count;
    /** @brief An array of binding layout for this set. */
    VkDescriptorSetLayoutBinding bindings[VULKAN_SHADER_MAX_BINDINGS];
    /** @brief The index of the sampler binding. */
    u8 sampler_binding_index;
} vulkan_descriptor_set_config;

/** @brief Internal shader configuration generated by vulkan_shader_create(). */
typedef struct vulkan_shader_config{
    /** @brief The number of shader stages in this shader. */
    u8 stage_count;
    /** @brief The configuration for every stage of this shader. */
    vulkan_shader_stage_config stages[VULKAN_SHADER_MAX_STAGES];
    /** @brief An array of descriptor pool sizes. */
    VkDescriptorPoolSize pool_sizes[2];

    /**
     * @brief The max number of descriptor sets that can be allocated from this shader.
     * Should typically be a decently high number.
     */
    u16 max_descriptor_set_count;

    /**
     * @brief The total number of descriptor sets configure for this shader.
     * Is 1 if only using global uniforms/samplers; otherwise 2.
     */
    u8 descriptor_set_count;
    /** @brief Descriptor sets, max of 2. Index 0=global, 1=instance */
    vulkan_descriptor_set_config descriptor_sets[2];

    /** @brief An array of attribute descriptions for this shader. */
    VkVertexInputAttributeDescription attributes[VULKAN_SHADER_MAX_ATTRIBUTES];

    /** @brief Face culling mode, provided by the front end. */
    face_cull_mode cull_mode;

} vulkan_shader_config;

typedef struct vulkan_descriptor_state {
    /** @brief The descriptor generation, per frame. */
    u8 generations[3];
    /** @brief The identifier, per frame. Typically used for texture ids. */
    u32 ids[3];
} vulkan_descriptor_state;

typedef struct vulkan_shader_descriptor_set_state{
    // Per frame
    VkDescriptorSet descriptor_sets[3];

    // Per descriptor
    vulkan_descriptor_state descriptor_states[VULKAN_SHADER_MAX_BINDINGS];
} vulkan_shader_descriptor_set_state;

/**
 * @brief The instance-level state for a shader
 */
typedef struct vulkan_shader_instance_state {
    /** @brief The instance id. INVALID_ID if not used. */
    u32 id;
    /** @brief The offset in bytes in the instance uniform buffer. */
    u64 offset;
    /** @brief A state for the descriptor set. */
    vulkan_shader_descriptor_set_state descriptor_set_state;

    /**
     * @brief Instance texture maps pointers, which are used during rendering. These
     * are set by calls to set_sampler.
     */
    struct texture_map** instance_texture_maps;
} vulkan_shader_instance_state;

typedef struct vulkan_shader{
   
    /** @brief The block of memory mapped to the uniform buffer. */
    void* mapped_uniform_buffer_block;
    /** @brief The shader identifier. */
    u32 id;
    /** @brief The configuration of the shader generated by vulkan_create_sahder(). */
    vulkan_shader_config config;
    /** @brief A pointer to the renderpass to be used with this shader */
    vulkan_renderpass* renderpass;

    /** @brief An array of stages (such as vertex and fragment) for this shader. Count is located in config. */
    vulkan_shader_stage stages[VULKAN_SHADER_MAX_STAGES];

    /** @brief The descriptor pool used to this shader. */
    VkDescriptorPool descriptor_pool;

    /** @brief Descriptor set layouts, max of 2. Index 0=global, 1=instance. */
    VkDescriptorSetLayout descriptor_set_layouts[2];
    /** @brief Global descriptor sets, one per frame. */
    VkDescriptorSet global_descriptor_sets[3];
    /** @brief The uniform buffer used by this shader. */
    vulkan_buffer uniform_buffer;

    /** @brief The pipeline associated with this shader. */
    vulkan_pipeline pipeline;

    /** @brief The instance states for all instances. @todo TODO: make dynamic */
    u32 instance_count;
    vulkan_shader_instance_state instance_states[VULKAN_MAX_MATERIAL_COUNT];

    /** @brief The number of global non-sampler uniforms. */
    u8 global_uniform_count;
    /** @brief The number of global sampler uniforms. */
    u8 global_uniform_sampler_count;
    /** @brief The number of instance non-sampler uniforms. */
    u8 instance_uniform_count;
    /** @brief The number of instance sampler uniforms. */
    u8 instance_uniform_sampler_count;
    /** @brief The number of local non-sampler uniforms. */
    u8 local_uniform_count;

} vulkan_shader;

#define VULKAN_MAX_REGISTERED_RENDERPASSES 31

typedef struct vulkan_context{
    f32 frame_delta_time;

    // The framebuffer's current width
    u32 framebuffer_width;

    // The framebuffer's current height
    u32 framebuffer_height;

    // Current generation of framebuffer size. If it does not match framebuffer_size_last_generation,
    // a new one should be generated.
    u64 framebuffer_size_generation;

    // The generation of the framebuffer when it was last created. Set to framebuffer_size_generation
    // when updated.
    u64 framebuffer_size_last_generation;

    VkInstance instance;
    VkAllocationCallbacks* allocator;
    VkSurfaceKHR surface;

    #if defined(_DEBUG)
        VkDebugUtilsMessengerEXT debug_messenger;
    #endif

    vulkan_device device;

    vulkan_swapchain swapchain;
    
    void* renderpass_table_block;
    hashtable renderpass_table;


    /** @brief Registered renderpasses. */
    renderpass registered_passes[VULKAN_MAX_REGISTERED_RENDERPASSES];

    vulkan_buffer object_vertex_buffer;
    vulkan_buffer object_index_buffer;

    // darray
    vulkan_command_buffer* graphics_command_buffers;

    // darray
    VkSemaphore* image_available_semaphores;

    // darray
    VkSemaphore* queue_complete_semaphores;

    u32 in_flight_fence_count;
    VkFence in_flight_fences[2];

    // Holds fences which exist and are owned elsewhere, one per frame.
    VkFence images_in_flight[3];

    u32 image_index;
    u32 current_frame;

    b8 recreating_swapchain;

    // TODO: make dynamic
    vulkan_geometry_data geometries[VULKAN_MAX_GEOMETRY_COUNT];

    /** @brief Render targets used for world rendering. @note One per frame. */
    render_target world_render_targets[3];

    i32 (*find_memory_index)(u32 type_filter, u32 property_flags);

    
    /**
     * @brief A pointer to a function to be called when the backend requires
     * rendertargets to be refreshed/regenerated.
     */
    void (*on_rendertarget_refresh_required)();
} vulkan_context;