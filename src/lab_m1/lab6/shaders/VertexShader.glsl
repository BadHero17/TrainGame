#version 330

// Input
// TODO(student): Get vertex attributes from each location
layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_text_coord;
layout(location = 3) in vec3 in_color;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform float elapsedTime;

// Output
// TODO(student): Output values to fragment shader
out vec3 f_color;

void main()
{
    // TODO(student): Send output to fragment shader
    f_color = in_color;

    // TODO(student): Compute gl_Position
    vec4 temp_pos = Model * glm::vec4(in_position, 1);
    temp_pos.xyz += 4 * sin(elapsedTime);
    gl_Position = Projection * View * Model * temp_pos;

}
