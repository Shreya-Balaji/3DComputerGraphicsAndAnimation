
#version 410 core

in vec2 TexCoord;

uniform sampler2D minimapTexture;

out vec4 FragColor;

void main()
{
    FragColor = texture(minimapTexture, TexCoord);
}
