#version 330 core

in float edgeFade;
out vec4 fragColor;

void main()
{
    float radius = length(gl_PointCoord - vec2(0.5));
    float alpha = 0.7 * edgeFade * (1.0 - smoothstep(0.25, 0.5, radius));
    if (alpha <= 0.0)
        discard;
    fragColor = vec4(1.0, 1.0, 1.0, alpha);
}
