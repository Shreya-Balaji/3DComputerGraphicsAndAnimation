#version 330 core

in float edgeFade;
in float distanceToCamera;
uniform bool fogEnabled;
uniform float fogStart;
uniform float fogEnd;
const vec3 FOG_COLOR = vec3(0.55, 0.65, 0.72);
out vec4 fragColor;

void main()
{
    float radius = length(gl_PointCoord - vec2(0.5));
    float alpha = 0.7 * edgeFade * (1.0 - smoothstep(0.25, 0.5, radius));
    if (alpha <= 0.0)
        discard;
    fragColor = vec4(1.0, 1.0, 1.0, alpha);
}
