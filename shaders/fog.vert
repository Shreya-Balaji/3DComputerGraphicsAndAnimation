#version 330 core

out vec2 texCoord;

void main()
{
    // One oversized triangle to cover the screen, the excess is clipped by OpenGL
    const vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    vec2 position = positions[gl_VertexID];
    gl_Position = vec4(position, 0.0, 1.0);

    // Convert screen coordinates [-1, 1] -> texture coordinates [0, 1]
    texCoord = position * 0.5 + 0.5;
}
