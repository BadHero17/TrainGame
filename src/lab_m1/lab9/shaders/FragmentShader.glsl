#version 330

// Input
in vec2 texcoord;

// Uniform properties
uniform sampler2D texture_1;
uniform sampler2D texture_2;
// TODO(student): Declare various other uniforms
uniform float elapsed_time;

// Output
layout(location = 0) out vec4 out_color;


void main()
{
    // TODO(student): Calculate the out_color using the texture2D() function.
    vec2 modified_texcoord = texcoord;
    modified_texcoord.x = fract(modified_texcoord.x + elapsed_time);

    vec4 color1 = texture2D(texture_1, texcoord);
    vec4 color2 = texture2D(texture_2, modified_texcoord);
    if (color1.a < 0.5) {
        discard;
    }
    out_color = mix(color1, color2, 0.5);

}
