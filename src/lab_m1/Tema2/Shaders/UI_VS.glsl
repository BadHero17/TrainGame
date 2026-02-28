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
    f_color = in_color;
    gl_Position = Model * vec4(in_position, 1);
}