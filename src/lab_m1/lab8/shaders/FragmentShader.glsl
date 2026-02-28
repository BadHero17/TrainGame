#version 330

// Input
in vec3 world_position;
in vec3 world_normal;

// Uniforms for light properties
uniform vec3 light_direction;
uniform vec3 light_position;
uniform vec3 light2_direction;
uniform vec3 light2_position;
uniform vec3 eye_position;

uniform float material_kd;
uniform float material_ks;
uniform int material_shininess;

// TODO(student): Declare any other uniforms
uniform int is_directional;
uniform vec3 object_color;
uniform float theta;

// Output
layout(location = 0) out vec4 out_color;


void main()
{
    // TODO(student): Define ambient, diffuse and specular light components
    float ambient_light = 0.2;
    float light_color = 1.0;
    vec3 L = normalize(light_position - world_position);
    vec3 V = normalize(eye_position - world_position);
    vec3 H = normalize(L + V);
    vec3 L2 = normalize(light2_position - world_position);
    vec3 H2 = normalize(L2 + V);
    float diffuse_light = material_kd * light_color * max(dot(world_normal, L), 0);
    float specular_light = 0;
    // It's important to distinguish between "reflection model" and
    // "shading method". In this shader, we are experimenting with the Phong
    // (1975) and Blinn-Phong (1977) reflection models, and we are using the
    // Phong (1975) shading method. Don't mix them up!
    if (diffuse_light > 0)
    {
        specular_light = material_ks * light_color * pow(max(dot(world_normal, H), 0), material_shininess);
    }

    // TODO(student): If (and only if) the light is a spotlight, we need to do
    // some additional things.

    float diffuse_light2 = material_kd * light_color * max(dot(world_normal, L2), 0);
    float specular_light2 = 0;
    if (diffuse_light2 > 0) {
        specular_light2 = material_ks * light_color * pow(max(dot(world_normal, H2), 0), material_shininess);
    }

    float cos_theta = cos(theta);

    if (is_directional == 1) {
        if (dot(-L, light_direction) < cos_theta) {
            specular_light = 0;
            diffuse_light = 0;
        }
        else {
            specular_light *= pow((dot(-L, light_direction) - cos_theta) / (1.0 - cos_theta), 4);
            diffuse_light *= pow((dot(-L, light_direction) - cos_theta) / (1.0 - cos_theta), 4);
        }
    }

    // TODO(student): Compute the total light. You can just add the components
    // together, but if you're feeling extra fancy, you can add individual
    // colors to the light components. To do that, pick some vec3 colors that
    // you like, and multiply them with the respective light components.
    float light = ambient_light + (diffuse_light2 + specular_light2 + diffuse_light + specular_light) * (1 / (length(L) * length (L)));

    // TODO(student): Write pixel out color
    out_color = vec4(light * object_color, 1);

}
