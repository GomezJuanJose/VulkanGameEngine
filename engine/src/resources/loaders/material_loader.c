#include "material_loader.h"

#include "core/logger.h"
#include "core/tmemory.h"
#include "core/tstring.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "math/tmath.h"

#include "platform/filesystem.h"

#include "loader_utils.h"

b8 material_loader_load(struct resource_loader* self, const char*name, resource* out_resource){
    if(!self || !name || !out_resource){
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    char full_file_path[512];
    string_format(full_file_path, format_str, resource_system_base_path(), self->type_path, name, ".tmt");

    file_handle f;
    if(!filesystem_open(full_file_path, FILE_MODE_READ, FALSE, &f)){
        TERROR("material_loader_load - unable to open material file for reading: '%s'.", full_file_path);
        return FALSE;
    }

    // TODO: Should be using an allocator here.
    out_resource->full_path = string_duplicate(full_file_path);

    // TODO: Should be using an allocator here.
    material_config* resource_data = tallocate(sizeof(material_config), MEMORY_TAG_MATERIAL_INSTANCE);
    // Set some defaults.
    resource_data->shader_name = "Builtin.Material"; // Default material.
    resource_data->auto_release = TRUE;
    resource_data->diffuse_colour = vec4_one(); // White
    resource_data->diffuse_map_name[0] = 0;
    resource_data->specular_map_name[0] = 0;
    resource_data->normal_map_name[0] = 0;
    string_ncopy(resource_data->name, name, MATERIAL_NAME_MAX_LENGTH);

    // Read each line of the file.
    char line_buf[512] = "";
    char* p = &line_buf[0];
    u64 line_length = 0;
    u32 line_number = 1;
    while(filesystem_read_line(&f, 511, &p, &line_length)){
        // Trim the string.
        char* trimmed = string_trim(line_buf);

        // Get the trimmed length.
        line_length = string_length(trimmed);

        // Skip blank lines and comments.
        if(line_length < 1 || trimmed[0] == '#'){
            line_number++;
            continue;
        }

        // Split into var/value
        i32 equal_index = string_index_of(trimmed, '=');
        if(equal_index == -1){
            TWARN("Potential formatting issue found in file '%s': '=' token not found. Skipping line %ui.", full_file_path, line_number);
            line_number++;
            continue;
        }

        // Assume a max of 64 characters for the variable name.
        char raw_var_name[64];
        tzero_memory(raw_var_name, sizeof(char) * 64);
        string_mid(raw_var_name, trimmed, 0, equal_index);
        char* trimmed_var_name = string_trim(raw_var_name);

        // Assume a max of 511-65 (446) for the max length of the value to account for the variable name and the '='.
        char raw_value[446];
        tzero_memory(raw_value, sizeof(char) * 446);
        string_mid(raw_value, trimmed, equal_index + 1, -1); // Read the rest of the line
        char* trimmed_value = string_trim(raw_value);

        // Process the varaible.
        if(strings_equali(trimmed_var_name, "version")){
            // TODO: version
        } else if (strings_equali(trimmed_var_name, "name")){
            string_ncopy(resource_data->name, trimmed_value, MATERIAL_NAME_MAX_LENGTH);
        } else if (strings_equali(trimmed_var_name, "diffuse_map_name")){
            string_ncopy(resource_data->diffuse_map_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        }else if (strings_equali(trimmed_var_name, "specular_map_name")){
            string_ncopy(resource_data->specular_map_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        }else if (strings_equali(trimmed_var_name, "normal_map_name")){
            string_ncopy(resource_data->normal_map_name, trimmed_value, TEXTURE_NAME_MAX_LENGTH);
        } else if (strings_equali(trimmed_var_name, "diffuse_colour")){
            // Parse the colour
            if(!string_to_vec4(trimmed_value, &resource_data->diffuse_colour)){
                TWARN("Error parsing diffuse_colour in file '%s'. Using default of white instead.", full_file_path);
                // NOTE: The default value was assigned above.
            }
        } else if (strings_equali(trimmed_var_name, "shader")){
            // Take a copy of the material name.
            resource_data->shader_name = string_duplicate(trimmed_value);
        } else if(strings_equali(trimmed_var_name, "shininess")){
            if(!string_to_f32(trimmed_value, &resource_data->shininess)){
                TWARN("Error parsing shininess in file '%s'. Using default of 32.0 instead.", full_file_path);
                resource_data->shininess = 32.0f;
            }
        }

        // TODO: more fields.

        // Clear the line buffer.
        tzero_memory(line_buf, sizeof(char) * 512);
        line_number++;
    }

    filesystem_close(&f);

    out_resource->data = resource_data;
    out_resource->data_size = sizeof(material_config);
    out_resource->name = name;

    return TRUE;
}

void material_loader_unload(struct resource_loader* self, resource* resource){
    if(!resource_unload(self, resource, MEMORY_TAG_MATERIAL_INSTANCE)){
        TWARN("material_loader_unload called with nullptr for self or resource.");
    }
}

resource_loader material_resource_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_MATERIAL;
    loader.custom_type = 0;
    loader.load = material_loader_load;
    loader.unload = material_loader_unload;
    loader.type_path = "materials";

    return loader;
}