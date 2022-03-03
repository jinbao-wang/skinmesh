#version 330 core

uniform mat4 mvpMat;
in vec4 attrPos;
in vec2 attrTexCoord;
out vec2 texture_coord;

void main()
{
    gl_Position = mvpMat * attrPos;
    texture_coord = attrTexCoord;
}
