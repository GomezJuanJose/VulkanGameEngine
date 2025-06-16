#version 450

layout(location = 0) out vec4 out_colour;

layout(set = 1, binding = 0) uniform local_uniform_object{
    vec4 diffuse_colour;
    float shininess;
} object_ubo;

struct directional_light {
    vec3 direction;
    vec4 colour;
};

struct point_light{
    vec3 position;
    vec4 colour;
    // Usually 1, make sure denominator gets smaller than 1
    float constant;
    // Reduces light intensity linearly
    float linear;
    // Makes the light fall off slower at longer distances.
    float quadratic;
};

// TODO: feed in from cpu
directional_light dir_light = {
    vec3(-0.57735, -0.57735, 0.57735),
    vec4(0.4, 0.4, 0.2, 1.0)
};

// TODO: feed in from cpu
point_light p_light_0 = {
    vec3(-5.5, 0.0, -5.5),
    vec4(0.0, 1.0, 0.0, 1.0),
    1.0,
    0.35,
    0.44
};


// TODO: feed in from cpu
point_light p_light_1 = {
    vec3(5.5, 0.0, -5.5),
    vec4(1.0, 0.0, 0.0, 1.0),
    1.0,
    0.35,
    0.44
};

// Samplers, diffuse, specular
const int SAMP_DIFFUSE = 0;
const int SAMP_SPECULAR = 1;
const int SAMP_NORMAL = 2;
layout(set = 1, binding = 1) uniform sampler2D samplers[3];

layout(location = 0) flat in int in_mode;

// Data Transfer Object
layout(location = 1) in struct dto{
    vec4 ambient;
    vec2 tex_coord;
    vec3 normal;
    vec3 view_position;
    vec3 frag_position;
    vec4 colour;
    vec4 tangent;
} in_dto;

mat3 TBN;

vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_direction);
vec4 calculate_point_light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction);

void main(){
    vec3 normal = in_dto.normal;
    vec3 tangent = in_dto.tangent.xyz;
    tangent = (tangent - dot(tangent, normal) * normal);
    vec3 bitangent = cross(in_dto.normal, in_dto.tangent.xyz) * in_dto.tangent.w;
    TBN = mat3(tangent, bitangent, normal);

    // Update the normal to use a sample from the normal map.
    vec3 localNormal = 2.0 * texture(samplers[SAMP_NORMAL], in_dto.tex_coord).rgb - 1.0;
    normal = normalize(TBN * localNormal);

    if(in_mode == 0 || in_mode == 1){
        vec3 view_direction = normalize(in_dto.view_position - in_dto.frag_position);

        out_colour = calculate_directional_light(dir_light, normal, view_direction);

        out_colour += calculate_point_light(p_light_0, normal, in_dto.frag_position, view_direction);
        out_colour += calculate_point_light(p_light_1, normal, in_dto.frag_position, view_direction);
    } else if(in_mode == 2){
        out_colour = vec4(abs(normal), 1.0);
    }
    

}

vec4 calculate_directional_light(directional_light light, vec3 normal, vec3 view_direction){
    float diffuse_factor = max(dot(normal, -light.direction), 0.0);

    vec3 half_direction = normalize(view_direction - light.direction);
    float specular_factor = pow(max(dot(half_direction, normal), 0.0), object_ubo.shininess);

    vec4 diff_samp = texture(samplers[SAMP_DIFFUSE], in_dto.tex_coord);
    vec3 ambient = vec3(in_dto.ambient * object_ubo.diffuse_colour);
    vec3 diffuse = vec3(light.colour * diffuse_factor);
    vec3 specular = vec3(light.colour * specular_factor);

    if(in_mode == 0){
        diffuse *= vec3(diff_samp);
        ambient *= vec3(diff_samp);
        specular *= texture(samplers[SAMP_SPECULAR], in_dto.tex_coord).rgb;
    }

    return vec4((ambient + diffuse + specular), diff_samp.a);
}

vec4 calculate_point_light(point_light light, vec3 normal, vec3 frag_position, vec3 view_direction){
    vec3 light_direction = normalize(light.position - frag_position);
    float diffuse_factor = max(dot(normal, light_direction), 0.0);

    vec3 reflect_direction = reflect(-light_direction, normal);
    float specular_factor = pow(max(dot(view_direction, reflect_direction), 0.0), object_ubo.shininess);

    // Calculate attenuation, or light fallof over distance.
    float distance = length(light.position - frag_position);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    vec4 ambient = in_dto.ambient;
    vec4 diffuse = light.colour * diffuse_factor;
    vec4 specular = light.colour * specular_factor;

    if(in_mode == 0){
        vec4 diff_samp = texture(samplers[SAMP_DIFFUSE], in_dto.tex_coord);
        diffuse *= diff_samp;
        ambient *= diff_samp;
        specular *= vec4(texture(samplers[SAMP_SPECULAR], in_dto.tex_coord).rgb, diffuse.a); 
    }

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}