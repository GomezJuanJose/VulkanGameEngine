#include "image_loader.h"

#include "core/logger.h"
#include "core/tmemory.h"
#include "core/tstring.h"

#include "platform/filesystem.h"

#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "platform/filesystem.h"
#include "loader_utils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "vendor/stb_image.h"


b8 image_loader_load(struct resource_loader* self, const char* name, resource* out_resource){
    if(!self || !name || !out_resource){
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    const i32 required_channel_count = 4;
    stbi_set_flip_vertically_on_load(TRUE);
    char full_file_path[512];

    // Try different extensions
    #define IMAGE_EXTENSION_COUNT 5
    b8 found = FALSE;
    char* extensions[IMAGE_EXTENSION_COUNT] = {".tga", ".png", ".jpg", ".bmp"};
    for(u32 i = 0; i < IMAGE_EXTENSION_COUNT; ++i){
        string_format(full_file_path, format_str, resource_system_base_path(), self->type_path, name, extensions[i]);
        if(filesystem_exists(full_file_path)){
            found = TRUE;
            break;
        }
    }

    if(!found){
        TERROR("Image resource loader failed find file '%s/%s/%s'.", resource_system_base_path(), self->type_path, name);
        return FALSE;
    }

    i32 width;
    i32 height;
    i32 channel_count;

    // For now, assume 8 bits per channel, 4 channels.
    // TODO: extend this to make it configurable.
    u8* data = stbi_load(full_file_path, &width, &height, &channel_count, required_channel_count);

    // Check for failure reason. If there is one, abort, clear memory if allocated, return FALSE.
    // NOTE: Commeted because it can lead to a false positive, even it loads the image.
    /*
    const char* fail_reason = stbi_failure_reason();
    if(fail_reason){
        TERROR("Image resource loader failed to load file '%s' : %s", full_file_path, fail_reason);
        // Clear the error so the next load doesn't fail.
        stbi__err(0, 0);

        if(!data){
            TERROR("Image resource loader failed to load file '%s'.", full_file_path);
            return FALSE;
        }
    }
    */
        
    // TODO: Should be using an allocator here.
    out_resource->full_path = string_duplicate(full_file_path);

    // TODO: Should be using an allocator here.
    image_resource_data* resource_data = tallocate(sizeof(image_resource_data), MEMORY_TAG_TEXTURE);
    resource_data->pixel = data;
    resource_data->width = width;
    resource_data->height = height;
    resource_data->channel_count = required_channel_count;

    out_resource->data = resource_data;
    out_resource->data_size = sizeof(image_resource_data);
    out_resource->name = name;

    return TRUE;
}

void image_loader_unload(struct resource_loader* self, resource* resource){
    if(!resource_unload(self, resource, MEMORY_TAG_TEXTURE)){
        TWARN("image_loader_unload called with nullptr for self or resource.");
    }
}

resource_loader image_resource_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_IMAGE;
    loader.custom_type = 0;
    loader.load = image_loader_load;
    loader.unload = image_loader_unload;
    loader.type_path = "textures";

    return loader;
}