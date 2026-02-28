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
    f_color = vec3(0, 0.3, 0.6);
    gl_Position = Projection * View * vec4(worldPos, 1.0);
}