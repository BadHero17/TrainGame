#version 330

// Input
// TODO(student): Get values from vertex shader
in vec3 f_color;

// Output
layout(location = 0) out vec4 out_color;

uniform float elapsedTime;

void main()
{
    // TODO(student): Write pixel out color
    out_color = vec4(max(min(f_color + 0.5 * sin(elapsedTime), 1.0), 0.0), 1);

}
