#version 330

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;

uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 lightPos;

out vec3 f_color;

void main()
{
    vec3 worldPos = vec3(Model * vec4(in_position, 1.0));
    mat3 normalMatrix = transpose(inverse(mat3(Model)));
    vec3 worldNormal = normalize(normalMatrix * in_normal);

    vec3 ambient = vec3(0.3);

    vec3 lightDir = normalize(lightPos - worldPos);
    float diff = max(dot(worldNormal, lightDir), 0.0);
    vec3 diffuse = 0.55 * diff * vec3(1.0);

    f_color = (ambient + diffuse) * in_color;
    gl_Position = Projection * View * vec4(worldPos, 1.0);
}