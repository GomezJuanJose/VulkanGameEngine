#include "binary_loader.h"

#include "core/logger.h"
#include "core/tmemory.h"
#include "core/tstring.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "math/tmath.h"

#include "platform/filesystem.h"
#include "loader_utils.h"

b8 binary_loader_load(struct resource_loader* self, const char* name, resource* out_resource){
    if(!self || !name || !out_resource){
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    char full_file_path[512];
    string_format(full_file_path, format_str, resource_system_base_path(), self->type_path, name, "");

    file_handle f;
    if(!filesystem_open(full_file_path, FILE_MODE_READ, TRUE, &f)){
        TERROR("binary_loader_load - unable to open file for binary reading: '%s'.", full_file_path);
        return FALSE;
    }

    // TODO: Should be using an allocator here.
    out_resource->full_path = string_duplicate(full_file_path);

    u64 file_size = 0;
    if(!filesystem_size(&f, &file_size)){
        TERROR("Unable to binary read file: %s.", full_file_path);
        filesystem_close(&f);
        return FALSE;
    }

    // TODO: Should be using an allocator here.
    u8* resource_data = tallocate(sizeof(u8) * file_size, MEMORY_TAG_ARRAY);
    u64 read_size = 0;
    if(!filesystem_read_all_bytes(&f, resource_data, &read_size)){
        TERROR("Unable to binary read file: %s.", full_file_path);
        filesystem_close(&f);
        return FALSE;
    }

    filesystem_close(&f);

    out_resource->data = resource_data;
    out_resource->data_size = read_size;
    out_resource->name = name;

    return TRUE;
}

void binary_loader_unload(struct resource_loader* self, resource* resource){
    if(!resource_unload(self, resource, MEMORY_TAG_ARRAY)){
        TWARN("binary_loader_unload called with nullptr for self or resource.");
    }
}

resource_loader binary_resource_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_BINARY;
    loader.custom_type = 0;
    loader.load = binary_loader_load;
    loader.unload = binary_loader_unload;
    loader.type_path = "";

    return loader;
}