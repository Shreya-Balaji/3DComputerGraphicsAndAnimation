#version 330 core

in vec2 texCoord;
out vec4 fragColor;

uniform sampler2D sceneColor;
uniform sampler2D sceneDepth;
uniform mat4 inverseProjection;
uniform bool fogEnabled;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 fogColor;

void main()
{
    vec4 color = texture(sceneColor, texCoord);
    float depth = texture(sceneDepth, texCoord).r;
    vec4 viewPosition = inverseProjection * vec4(texCoord * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    float distanceToCamera = length(viewPosition.xyz / viewPosition.w);
    float amount = fogEnabled
        ? smoothstep(fogStart, max(fogEnd, fogStart + 0.001), distanceToCamera) : 0.0;
    fragColor = vec4(mix(color.rgb, fogColor, amount), color.a);
}
