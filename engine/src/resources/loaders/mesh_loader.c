#include "mesh_loader.h"

#include "core/logger.h"
#include "core/tmemory.h"
#include "core/tstring.h"
#include "containers/darray.h"
#include "resources/resource_types.h"
#include "systems/resource_system.h"
#include "systems/geometry_system.h"
#include "math/tmath.h"
#include "math/geometry_utils.h"
#include "loader_utils.h"

#include "platform/filesystem.h"

#include <stdio.h> //sscanf

typedef enum mesh_file_type {
    MESH_FILE_TYPE_NOT_FOUND,
    MESH_FILE_TYPE_TSM,
    MESH_FILE_TYPE_OBJ
} mesh_file_type;

typedef struct supported_mesh_filetype{
    char* extension;
    mesh_file_type type;
    b8 is_binary;
}supported_mesh_filetype;

typedef struct mesh_vertex_index_data{
    u32 position_index;
    u32 normal_index;
    u32 texcoord_index;
} mesh_vertex_index_data;

typedef struct mesh_face_data {
    mesh_vertex_index_data vertices[3];
} mesh_face_data;

typedef struct mesh_group_data {
    //darray
    mesh_face_data* faces;
} mesh_group_data;

b8 import_obj_file(file_handle* obj_file, const char* out_tsm_filename, geometry_config** out_geometries_darray);
void process_subobjects(vec3* positions, vec3* normals, vec2* tex_coords, mesh_face_data* faces, geometry_config* out_data);
b8 import_obj_material_library_file(const char* mtl_file_path);

b8 load_tsm_file(file_handle* tsm_file, geometry_config** out_geometries_darray);
b8 write_tsm_file(const char* path, const char* name, u32 geometry_count, geometry_config* geometries);
b8 write_tmt_file(const char* directory, material_config* config);

b8 mesh_loader_load(struct resource_loader* self, const char* name, resource* out_resource){
    if(!self || !name || !out_resource){
        return FALSE;
    }

    char* format_str = "%s/%s/%s%s";
    file_handle f;
    // Suported extensions. Note that these are in order of priority when looked up.
    // This is to prioritize the loading of a binary version of the mesh, followed by
    // importing various types of meshes to binary types, which would be loaded on the 
    // next run.
    // TODO: Might be good to be able to specify an override to always import (i.e. skip
    // binary versions) for debug purposes.
    #define SUPPORTED_FILETYPE_COUNT 2
    supported_mesh_filetype supported_filetypes[SUPPORTED_FILETYPE_COUNT];
    supported_filetypes[0] = (supported_mesh_filetype){".tsm", MESH_FILE_TYPE_TSM, TRUE};
    supported_filetypes[1] = (supported_mesh_filetype){".obj", MESH_FILE_TYPE_OBJ, FALSE};

    char full_file_path[512];
    mesh_file_type type = MESH_FILE_TYPE_NOT_FOUND;
    // Try each supported extension.
    for(u32 i = 0; i < SUPPORTED_FILETYPE_COUNT; ++i){
        string_format(full_file_path, format_str, resource_system_base_path(), self->type_path, name, supported_filetypes[i].extension);
        // If the file exists, open it and stop looking.
        if(filesystem_exists(full_file_path)){
            if(filesystem_open(full_file_path, FILE_MODE_READ, supported_filetypes[i].is_binary, &f)){
                type = supported_filetypes[i].type;
                break;
            }
        }
    }

    if(type == MESH_FILE_TYPE_NOT_FOUND){
        TERROR("Unable to find mesh of supported type called '%s'.", name);
        return FALSE;
    }

    out_resource->full_path = string_duplicate(full_file_path);

    // The resource data is just and array of configs.
    geometry_config* resource_data = darray_create(geometry_config);

    b8 result = FALSE;
    switch(type){
        case MESH_FILE_TYPE_OBJ: {
            // Generate the tsm filename.
            char tsm_file_name[512];
            string_format(tsm_file_name, "%s/%s/%s%s", resource_system_base_path(), self->type_path, name, ".tsm");
            result = import_obj_file(&f, tsm_file_name, &resource_data);
            break;
        }
        
        case MESH_FILE_TYPE_TSM:
            result = load_tsm_file(&f, &resource_data);
            break;
        
        default:
        case MESH_FILE_TYPE_NOT_FOUND:
            TERROR("Unable to find mesh of supported type called '%s'.", name);
            result = FALSE;
            break;
    }

    filesystem_close(&f);

    if(!result){
        TERROR("Failed to process mesh file '%s'.", full_file_path);
        darray_destroy(resource_data);
        out_resource->data = 0;
        out_resource->data_size = 0;
        return FALSE;
    }

    out_resource->data = resource_data;
    // Use the data size as count
    out_resource->data_size = darray_length(resource_data);

    return TRUE;
}

void mesh_loader_unload(struct resource_loader* self, resource* resource){
    u32 count = darray_length(resource->data);
    for(u32 i = 0; i < count; ++i){
        geometry_config* config = &((geometry_config*)resource->data)[i];
        geometry_system_config_dispose(config);
    }
    darray_destroy(resource->data);
    resource->data = 0;
    resource->data_size = 0;
}

b8 load_tsm_file(file_handle* tsm_file, geometry_config** out_geometries_darray){
    // Version
    u64 bytes_read = 0;
    u16 version = 0;
    filesystem_read(tsm_file, sizeof(u16), &version, &bytes_read);

    // Name length
    u32 name_length = 0;
    filesystem_read(tsm_file, sizeof(u32), &name_length, &bytes_read);
    // Name + terminator
    char name[256];
    filesystem_read(tsm_file, sizeof(char) * name_length, name, &bytes_read);

    // Geometry count
    u32 geometry_count = 0;
    filesystem_read(tsm_file, sizeof(u32), &geometry_count, &bytes_read);

    // Each geometry
    for (u32 i = 0; i < geometry_count; ++i){
        geometry_config g = {};

        // Vertices (size/count/array)
        filesystem_read(tsm_file, sizeof(u32), &g.vertex_size, &bytes_read);
        filesystem_read(tsm_file, sizeof(u32), &g.vertex_count, &bytes_read);
        g.vertices = tallocate(g.vertex_size * g.vertex_count, MEMORY_TAG_ARRAY);
        filesystem_read(tsm_file, g.vertex_size * g.vertex_count, g.vertices, &bytes_read);

        // Indices (size/count/array)
        filesystem_read(tsm_file, sizeof(u32), &g.index_size, &bytes_read);
        filesystem_read(tsm_file, sizeof(u32), &g.index_count, &bytes_read);
        g.indices = tallocate(g.index_size * g.index_count, MEMORY_TAG_ARRAY);
        filesystem_read(tsm_file, g.index_size * g.index_count, g.indices, &bytes_read);

        // Name
        u32 g_name_length = 0;
        filesystem_read(tsm_file, sizeof(u32), &g_name_length, &bytes_read);
        filesystem_read(tsm_file, sizeof(char) * g_name_length, g.name, &bytes_read);

        // Material Name
        u32 m_name_length = 0;
        filesystem_read(tsm_file, sizeof(u32), &m_name_length, &bytes_read);
        filesystem_read(tsm_file, sizeof(char) * m_name_length, g.material_name, &bytes_read);

        // Center
        filesystem_read(tsm_file, sizeof(vec3), &g.center, &bytes_read);

        // Extents (min/max)
        filesystem_read(tsm_file, sizeof(vec3), &g.min_extents, &bytes_read);
        filesystem_read(tsm_file, sizeof(vec3), &g.max_extents, &bytes_read);

        // Add to the output array.
        darray_push(*out_geometries_darray, g);
    }

    filesystem_close(tsm_file);

    return TRUE;
}

b8 write_tsm_file(const char* path, const char* name, u32 geometry_count, geometry_config* geometries){
    if(filesystem_exists(path)){
        TINFO("File '%s' already exists and will be overwritten.", path);
    }

    file_handle f;
    if(!filesystem_open(path, FILE_MODE_WRITE, TRUE, &f)){
        TERROR("Unable to open file '%s' for writing. TSM write failed.", path);
    }

    // Version
    u64 written = 0;
    u16 version = 0x0001U;
    filesystem_write(&f, sizeof(u16), &version, &written);

    // Name length
    u32 name_length = string_length(name) + 1;
    filesystem_write(&f, sizeof(u32), &name_length, &written);
    // Name + terminator
    filesystem_write(&f, sizeof(char) * name_length, name, &written);

    // Geometry count
    filesystem_write(&f, sizeof(u32), &geometry_count, &written);

    // Each geometry
    for(u32 i = 0; i < geometry_count; ++i){
        geometry_config* g = &geometries[i];

        // Vertices (size/count/array)
        filesystem_write(&f, sizeof(u32), &g->vertex_size, &written);
        filesystem_write(&f, sizeof(u32), &g->vertex_count, &written);
        filesystem_write(&f, g->vertex_size * g->vertex_count, g->vertices, &written);

        // Indices (size/count/array)
        filesystem_write(&f, sizeof(u32), &g->index_size, &written);
        filesystem_write(&f, sizeof(u32), &g->index_count, &written);
        filesystem_write(&f, g->index_size * g->index_count, g->indices, &written);

        // Name
        u32 g_name_length = string_length(g->name) + 1;
        filesystem_write(&f, sizeof(u32), &g_name_length, &written);
        filesystem_write(&f, sizeof(char) * g_name_length, g->name, &written);

        // Material Name
        u32 m_name_length = string_length(g->material_name) + 1;
        filesystem_write(&f, sizeof(u32), &m_name_length, &written);
        filesystem_write(&f, sizeof(char) * m_name_length, g->material_name, &written);

        // Center
        filesystem_write(&f, sizeof(vec3), &g->center, &written);

        // Extents (min/max)
        filesystem_write(&f, sizeof(vec3), &g->min_extents, &written);
        filesystem_write(&f, sizeof(vec3), &g->max_extents, &written);
    }

    filesystem_close(&f);
    
    return TRUE;
}

/**
 * @brief Imports an obj file. This reads the obj, creates geometry configs, then calls logic to write
 * those geometries out to a binary tsm file. That file can be used on the next load.
 * 
 * @param obj_file A pointer to the obj file handle to be read.
 * @param out_tsm_filename The path to the tsm file to be written to.
 * @param out_geometries_darray A darray of geometries parsed from the file.
 * @return True on success; otherwise false. 
 */
b8 import_obj_file(file_handle* obj_file, const char* out_tsm_filename, geometry_config** out_geometries_darray){
    // Positions
    vec3* positions = darray_reserve(vec3, 16384);

    // Normals
    vec3* normals = darray_reserve(vec3, 16384);

    // Texture coordinates
    vec2* tex_coords = darray_reserve(vec2, 16384);

    // Groups
    mesh_group_data* groups = darray_reserve(mesh_group_data, 4);

    char material_file_name[512] = "";

    char name[512];
    u8 current_mat_name_count = 0;
    char material_names[32][64];

    char line_buf[512] = "";
    char* p = &line_buf[0];
    u64 line_length = 0;

    // Index 0 is previous, 1 is previous before that.
    char prev_first_chars[2] = {0, 0};
    while (TRUE)
    {
        // Is not inside the while for better debug reasons
        if (!filesystem_read_line(obj_file, 511, &p, &line_length))
        {
            break;
        }

        // Skip blank lines.
        if(line_length < 1){
            continue;
        }

        char first_char = line_buf[0];

        switch(first_char){
            case '#':
                // Skip comments
                continue;

            case 'v':{
                char second_char = line_buf[1];
                switch(second_char){
                    case ' ': {
                        // Vertex position
                        vec3 pos;
                        char t[2];
                        sscanf(
                            line_buf,
                            "%s %f %f %f",
                            t,
                            &pos.x,
                            &pos.y,
                            &pos.z
                        );

                        darray_push(positions, pos);
                    }break;

                    case 'n':{
                        // Vertex normal
                        vec3 norm;
                        char t[2];
                        sscanf(
                            line_buf,
                            "%s %f %f %f",
                            t,
                            &norm.x,
                            &norm.y,
                            &norm.z
                        );

                        darray_push(normals, norm);
                    }break;

                    case 't': {
                        // Vertex texture coords.
                        vec2 tex_coord;
                        char t[2];

                        // NOTE: Ignoring Z if present.
                        sscanf(
                            line_buf,
                            "%s %f %f",
                            t,
                            &tex_coord.x,
                            &tex_coord.y
                        );

                        darray_push(tex_coords, tex_coord)
                    }break;
                }
            }break;

            case 's':{
            }break;

            case 'f':{
                // Face
                // f 1/1/1 2/2/2 3/3/3 = pos/tex/norm pos/tex/norm pos/tex/norm
                // f 1//1 2//2 3//3 = pos//norm pos//norm pos//norm
                // f 1 2 3 = pos pos pos
                mesh_face_data face;
                char t[2];
                
                u64 normal_count = darray_length(normals);
                u64 tex_coord_count = darray_length(tex_coords);

                if (normal_count >= 0 && tex_coord_count == 0){
                    sscanf(
                        line_buf,
                        "%s %d//%d %d//%d %d//%d",
                        t,
                        &face.vertices[0].position_index,
                        &face.vertices[0].normal_index,

                        &face.vertices[1].position_index,
                        &face.vertices[1].normal_index,

                        &face.vertices[2].position_index,
                        &face.vertices[2].normal_index
                    );
                }else if(normal_count == 0 || tex_coord_count == 0){
                    sscanf(
                        line_buf,
                        "%s %d %d %d",
                        t,
                        &face.vertices[0].position_index,
                        &face.vertices[1].position_index,
                        &face.vertices[2].position_index
                    );
                } else {
                    sscanf(
                        line_buf,
                        "%s %d/%d/%d %d/%d/%d %d/%d/%d",
                        t,
                        &face.vertices[0].position_index,
                        &face.vertices[0].texcoord_index,
                        &face.vertices[0].normal_index,

                        &face.vertices[1].position_index,
                        &face.vertices[1].texcoord_index,
                        &face.vertices[1].normal_index,

                        &face.vertices[2].position_index,
                        &face.vertices[2].texcoord_index,
                        &face.vertices[2].normal_index
                    );
                }
                u64 group_index = darray_length(groups) - 1;
                darray_push(groups[group_index].faces, face);
            }break;
            
            case 'm': {
                // Material library file.
                char substr[7];
                sscanf(
                    line_buf,
                    "%s %s",
                    substr,
                    material_file_name
                );
                
                // If found, save off the material file name.
                if (strings_nequali(substr, "mtllib", 6)){
                    // TODO: verification
                }
            }break;

            case 'g': {
                // case '0': {
                // New object. process the previous object first if we previously read anything in. This will only be true after the first object...
                // if (hit_name){
                u64 group_count = darray_length(groups);

                // Process each group as a subobject.
                for(u64 i = 0; i < group_count; ++i){
                    geometry_config new_data = {};
                    string_ncopy(new_data.name, name, 255);
                    if(i > 0){
                        string_append_int(new_data.name, new_data.name, i);
                    }
                    string_ncopy(new_data.material_name, material_names[i], 255);

                    process_subobjects(positions, normals, tex_coords, groups[i].faces, &new_data);
                    new_data.vertex_count = darray_length(new_data.vertices);
                    new_data.vertex_size = sizeof(vertex_3d);
                    new_data.index_count = darray_length(new_data.indices);
                    new_data.index_size = sizeof(u32);

                    darray_push(*out_geometries_darray, new_data);

                    // Increment the number of objects.
                    darray_destroy(groups[i].faces);
                    tzero_memory(material_names[i], 64);
                }
                current_mat_name_count = 0;
                darray_clear(groups);
                tzero_memory(name, 512);
                //}

                // hit_name = TRUE;

                // Read the name
                char t[2];
                sscanf(line_buf, "%s %s", t, name);

            }break;

            case 'u': {
                // Any time there is a usemtl, assume a new group.
                // New named group or moothing group, all faces coming after should be added to it.
                mesh_group_data new_group;
                new_group.faces = darray_reserve(mesh_face_data, 16384);
                darray_push(groups, new_group);

                // usemtl
                // Read the material name.
                char t[8];
                sscanf(line_buf, "%s %s", t, material_names[current_mat_name_count]);
                current_mat_name_count++;
            }break;
        }
        prev_first_chars[1] = prev_first_chars[0];
        prev_first_chars[0] = first_char;
    } // each line

    // Process the remaining group since the last one will not have been trigged
    // by the finding of a new name.
    // Process each group as a subobject.
    u64 group_count = darray_length(groups);
    for(u64 i = 0; i < group_count; ++i){
        geometry_config new_data = {};
        string_ncopy(new_data.name, name, 255);
        if(i > 0){
            string_append_int(new_data.name, new_data.name, i);
        }
        string_ncopy(new_data.material_name, material_names[i], 255);

        process_subobjects(positions, normals, tex_coords, groups[i].faces, &new_data);
        new_data.vertex_count = darray_length(new_data.vertices);
        new_data.vertex_size = sizeof(vertex_3d);
        new_data.index_count = darray_length(new_data.indices);
        new_data.index_size = sizeof(u32);

        darray_push(*out_geometries_darray, new_data);

        // Increment the number of objects.
        darray_destroy(groups[i].faces);
    }

    darray_destroy(groups);
    darray_destroy(positions);
    darray_destroy(normals);
    darray_destroy(tex_coords);
    
    if(string_length(material_file_name) > 0){
        // Load up the material file
        char full_mtl_path[512];
        string_directory_from_path(full_mtl_path, out_tsm_filename);
        string_append_string(full_mtl_path, full_mtl_path, material_file_name);

        // Process material library file.
        if(!import_obj_material_library_file(full_mtl_path)){
            TERROR("Error reading obj mtl file.");
        }
    }

    // De-duplicate geometry
    u32 count = darray_length(*out_geometries_darray);
    for(u64 i = 0; i < count; ++i){
        geometry_config* g = &((*out_geometries_darray)[i]);
        TDEBUG("geometry de-duplication process starting on geometry named '%s'...", g->name);

        u32 new_vert_count = 0;
        vertex_3d* unique_verts = 0;
        geometry_deduplicate_vertices(g->vertex_count, g->vertices, g->index_count, g->indices, &new_vert_count, &unique_verts);

        // Destroy the old, large array...
        darray_destroy(g->vertices);

        // And replace with the de-duplicated one.
        g->vertices = unique_verts;
        g->vertex_count = new_vert_count;

        // Take a copy of the indices as a normal, non-darray
        u32* indices = tallocate(sizeof(u32) * g->index_count, MEMORY_TAG_ARRAY);
        tcopy_memory(indices, g->indices, sizeof(u32) * g->index_count);
        // Destroy the darry
        darray_destroy(g->indices);
        // Replace with the non-darray version.
        g->indices = indices;

        // Also generate tangents here, this way tangents are also stored in the output file.
        geometry_generate_tangents(g->vertex_count, g->vertices, g->index_count, g->indices);
    }

    // Output a tsm file, which will be loaded in the future.
    return write_tsm_file(out_tsm_filename, name, count, *out_geometries_darray);
}

void process_subobjects(vec3* position, vec3* normals, vec2* tex_coords, mesh_face_data* faces, geometry_config* out_data){
    out_data->indices = darray_create(u32);
    out_data->vertices = darray_create(vertex_3d);
    b8 extent_set = FALSE;
    tzero_memory(&out_data->min_extents, sizeof(vec3));
    tzero_memory(&out_data->max_extents, sizeof(vec3));

    u64 face_count = darray_length(faces);
    u64 normal_count = darray_length(normals);
    u64 tex_coord_count = darray_length(tex_coords);
    
    b8 skip_normals = FALSE;
    b8 skip_tex_coords = FALSE;
    if(normal_count == 0){
        TWARN("No normals are present in this model.");
        skip_normals = TRUE;
    }
    if(tex_coords == 0){
        TWARN("No texture coordinates are present in this model.");
        skip_tex_coords = TRUE;
    }
    for(u64 f = 0; f < face_count; ++f){
        mesh_face_data face = faces[f];

        // Each vertex
        for(u64 i = 0; i < 3; ++i){
            mesh_vertex_index_data index_data = face.vertices[i];
            darray_push(out_data->indices, (u32)(i + (f * 3)));

            vertex_3d vert;

            vec3 pos = position[index_data.position_index - 1];
            vert.position = pos;

            // Check extents - min
            if(pos.x < out_data->min_extents.x || !extent_set){
                out_data->min_extents.x = pos.x;
            }
            if(pos.y < out_data->min_extents.y || !extent_set){
                out_data->min_extents.y = pos.y;
            }
            if(pos.z < out_data->min_extents.z || !extent_set){
                out_data->min_extents.z = pos.z;
            }

            // Check extents - max
            if(pos.x > out_data->max_extents.x || !extent_set){
                out_data->max_extents.x = pos.x;
            }
            if(pos.y > out_data->max_extents.y || !extent_set){
                out_data->max_extents.y = pos.y;
            }
            if(pos.z > out_data->max_extents.z || !extent_set){
                out_data->max_extents.z = pos.z;
            }

            extent_set = TRUE;

            if(skip_normals){
                vert.normal = vec3_create(0, 0, 1);
            }else{
                vert.normal = normals[index_data.normal_index - 1];
            }

            if(skip_tex_coords){
                vert.texcoord = vec2_zero();
            } else {
                vert.texcoord = tex_coords[index_data.texcoord_index - 1];
            }

            // TODO: Color. Hardcode to white for now.
            vert.colour = vec4_one();

            darray_push(out_data->vertices, vert);
        }
    }

    for(u8 i = 0; i < 3; ++i){
        out_data->center.elements[i] = (out_data->min_extents.elements[i] + out_data->max_extents.elements[i]) / 2.0f;
    }

}

// TODO: Load the material library file, and create material definitions from it.
// These definitions should be output to .tmt files. These tmt files are then
// loaded when the material is acquired on mesh load.
// NOTE: This should eventually account for duplicate materials. When the .tmt
// files are written, if the file already exists the material should have something
// such as a number appended to its name and a warning thrown to the console. The artist
// should make sure material names are unique. When the material is acquired, the _original_
// existing material name would be used, which would visially be wrong an serve as additional
// reinforcement of the message for material uniqueness.
// Material configs should not be returned or used here.
b8 import_obj_material_library_file(const char* mtl_file_path){
    TDEBUG("Importing obj .mtl file '%s'...", mtl_file_path);
    // Grab the .mtl file, if exists, and read the material information.
    file_handle mtl_file;
    if(!filesystem_open(mtl_file_path, FILE_MODE_READ, FALSE, &mtl_file)){
        TERROR("Unable to open mtl file: %s", mtl_file_path);
        return FALSE;
    }

    material_config current_config;
    tzero_memory(&current_config, sizeof(current_config));

    b8 hit_name = FALSE;

    char* line = 0;
    char line_buffer[512];
    char* p = &line_buffer[0];
    u64 line_length = 0;
    while(TRUE){
        if(!filesystem_read_line(&mtl_file, 512, &p, &line_length)){
            break;
        }
        // Trim the line first.
        line = string_trim(line_buffer);
        line_length = string_length(line);

        // Skip blank lines
        if(line_length < 1){
            continue;
        }

        char first_char = line[0];
        switch(first_char){
            case '#':
                // Skips comments
                continue;
            
            case 'K':{
                char second_char = line[1];
                switch(second_char){
                    case 'a':
                    case 'd':{
                        // Ambient/Diffuse colour are treated the same at this level.
                        // ambient colour is determined by the level.
                        char t[2];
                        sscanf(
                            line,
                            "%s %f %f %f",
                            t,
                            &current_config.diffuse_colour.r,
                            &current_config.diffuse_colour.g,
                            &current_config.diffuse_colour.b
                        );

                        // NOTE: This is only used by the colour shader, and will set to max_norm by default.
                        // Transparency could be added as a material property all its own at a later time.
                        current_config.diffuse_colour.a = 1.0f;
                    }break;

                    case 's':{
                        // Specular colout
                        char t[2];

                        // NOTE: Not using this for now.
                        f32 spec_rubbish = 0.0f;
                        sscanf(
                            line,
                            "%s %f %f %f",
                            t,
                            &spec_rubbish,
                            &spec_rubbish,
                            &spec_rubbish
                        );
                    }break;
                }
            }break;

            case 'N': {
                char second_char = line[1];
                
                switch(second_char){
                    case 's': {
                        // Specular exponent
                        char t[2];
                        sscanf(line, "%s %f", t, &current_config.shininess);
                    }break;
                }
            }break;

            case 'm':{
                // map
                char substr[10];
                char texture_file_name[512];

                sscanf(
                    line,
                    "%s %s",
                    substr,
                    texture_file_name
                );

                if(strings_nequali(substr, "map_Kd", 6)){
                    // Is a diffuse texture map
                    string_filename_no_extension_from_path(current_config.diffuse_map_name, texture_file_name);
                } else if(strings_nequali(substr, "map_Ks", 6)){
                    // Is a specular texture map
                    string_filename_no_extension_from_path(current_config.specular_map_name, texture_file_name);
                } else if(strings_nequali(substr, "map_bump", 8)){
                    // Is a bump texture map
                    string_filename_no_extension_from_path(current_config.normal_map_name, texture_file_name);
                }
            }break;

            case 'b': {
                // Some implementations use 'bump' instead of 'map_bump'.
                char substr[10];
                char texture_file_name[512];

                sscanf(
                    line,
                    "%s %s",
                    substr,
                    texture_file_name
                );

                if(strings_nequali(substr, "bump", 4)){
                    // Is a bump (normal) texture map
                    string_filename_no_extension_from_path(current_config.normal_map_name, texture_file_name);
                }
            }

            case 'n': {

                char substr[10];
                char material_name[512];

                sscanf(
                    line,
                    "%s %s",
                    substr,
                    material_name
                );

                if(strings_nequali(substr, "newmtl", 6)){
                    // Is a material name.

                    // NOTE: Hardcoding default material shader name beacuse all objects imported this way
                    // will be treated the same.
                    current_config.shader_name = "Shader.Builtin.Material";
                    // NOTE: Shininess of 0 will cause problems in the shader. Use a default
                    // if this is the case.
                    if(current_config.shininess == 0.0f){
                        current_config.shininess = 8.0f;
                    }
                    if(hit_name){
                        // Write out a tmt file and move on.
                        if(!write_tmt_file(mtl_file_path, &current_config)){
                            TERROR("Unable to write tmt file.");
                            return FALSE;
                        }

                        // Reset the material for the next round.
                        tzero_memory(&current_config, sizeof(current_config));
                    }

                    hit_name = TRUE;

                    string_ncopy(current_config.name, material_name, 256);
                }
            }
        }
    } // each line

    // Write out the remaining tmt file.
    // NOTE: Hardcoding default material shader name because all objects imported this way
    // will be treated the same.
    current_config.shader_name = "Shader.Builtin.Material";
    // NOTE: Shininess of 0 will cause problems in the shader. Use a default
    // if this is the case.
    if(current_config.shininess == 0.0f){
        current_config.shininess = 8.0f;
    }
    if(!write_tmt_file(mtl_file_path, &current_config)){
        TERROR("Unable to write tmt file.");
        return FALSE;
    }

    filesystem_close(&mtl_file);
    return TRUE;
}

/**
 * @brief Write out a Taller material file from config. This gets loaded by name later when the mesh
 * is requested for load.
 * 
 * @param mtl_file_path The filepath of the material library file which originally contained the material definition.
 * @param config A pointer to the config to be converted to tmt.
 * @return True on success; otherwise false.
 */
b8 write_tmt_file(const char* mtl_file_path, material_config* config){
    // NOTE: The .obj file this came from (and resulting .mtl file) sit in the
    // model directory. This moves up a level and back into the materials folder.
    // TODO: Read from config and get an absolute path for output.
    char* format_str = "%s../materials/%s%s";
    file_handle f;
    char directory[320];
    string_directory_from_path(directory, mtl_file_path);
    char full_file_path[512];

    string_format(full_file_path, format_str, directory, config->name, ".tmt");
    if(!filesystem_open(full_file_path, FILE_MODE_WRITE, FALSE, &f)){
        TERROR("Error opening material file for writing: '%s'", full_file_path);
        return FALSE;
    }
    TDEBUG("Writing .tmt file '%s'...", full_file_path);

    char line_buffer[512];
    filesystem_write_line(&f, "#material file");
    filesystem_write_line(&f, "");
    filesystem_write_line(&f, "version=0.1");
    string_format(line_buffer, "name=%s", config->name);
    filesystem_write_line(&f, line_buffer);
    string_format(line_buffer, "diffuse_colour=%.6f %.6f %.6f %.6f", config->diffuse_colour.r, config->diffuse_colour.g, config->diffuse_colour.b, config->diffuse_colour.a);
    filesystem_write_line(&f, line_buffer);
    string_format(line_buffer, "shininess=%.6f", config->shininess);
    filesystem_write_line(&f, line_buffer);

    if(config->diffuse_map_name[0]){
        string_format(line_buffer, "diffuse_map_name=%s", config->diffuse_map_name);
        filesystem_write_line(&f, line_buffer);
    }
    if(config->specular_map_name[0]){
        string_format(line_buffer, "specular_map_name=%s", config->specular_map_name);
        filesystem_write_line(&f, line_buffer);
    }
    if(config->normal_map_name[0]){
        string_format(line_buffer, "normal_map_name=%s", config->normal_map_name);
        filesystem_write_line(&f, line_buffer);
    }
    string_format(line_buffer, "shader=%s", config->shader_name);
    filesystem_write_line(&f, line_buffer);

    filesystem_close(&f);

    return TRUE;
}

resource_loader mesh_resource_loader_create(){
    resource_loader loader;
    loader.type = RESOURCE_TYPE_MESH;
    loader.custom_type = 0;
    loader.load = mesh_loader_load;
    loader.unload = mesh_loader_unload;
    loader.type_path = "models";

    return loader;
}