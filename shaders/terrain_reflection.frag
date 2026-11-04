#version 330 core

in vec3 snowColor;
out vec4 fragColor;

void main()
{
    fragColor = vec4(snowColor, 1.0);
}
