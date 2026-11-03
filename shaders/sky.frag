#version 330 core
in vec2 ndc;

uniform mat4 invViewProj;
uniform vec3 cameraPos;
uniform vec3 toSun; 
uniform vec3 toMoon;
uniform vec3 horizonColor;
uniform vec3 sunColor;
uniform vec3 moonColor;

out vec4 FragColor;

void main()
{
    vec4 world = invViewProj * vec4(ndc, 1.0, 1.0); 
    vec3 dir = normalize(world.xyz / world.w - cameraPos);
    vec3 zenithColor = horizonColor * 0.45;
    vec3 sky = mix(horizonColor, zenithColor, pow(clamp(dir.y, 0.0, 1.0), 0.5));
    vec3 groundColor = horizonColor * 0.6;
    sky = mix(groundColor, sky, smoothstep(-0.08, 0.02, dir.y));
    float sunDot = max(dot(dir, normalize(toSun)), 0.0);
    sky += sunColor * (pow(sunDot, 1500.0) * 6.0 + pow(sunDot, 12.0) * 0.25);

    float moonDot = max(dot(dir, normalize(toMoon)), 0.0);
    sky += moonColor * (pow(moonDot, 2500.0) * 4.0 + pow(moonDot, 20.0) * 0.1);

    FragColor = vec4(sky, 1.0);
}
