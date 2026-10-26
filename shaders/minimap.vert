
#version 410 core

layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
    TexCoord = aTexCoord;

    // Convert [0, 1] quad coordinates to [-1, 1] clip coordinates.
    vec2 clipPosition = aPosition * 2.0 - 1.0;
    gl_Position = vec4(clipPosition, 0.0, 1.0);
}
