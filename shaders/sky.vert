#version 330 core
out vec2 ndc;

void main()
{
    vec2 p = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
    ndc = p * 2.0 - 1.0;                                    
    gl_Position = vec4(ndc, 0.0, 1.0);
}
