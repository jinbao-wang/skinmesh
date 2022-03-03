#version 330 core

in vec2 texture_coord;
uniform sampler2D samp;
out vec4 color;

void main()
{
    color = vec4(texture(samp, texture_coord));
}

