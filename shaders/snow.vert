#version 330 core

layout(location = 0) in vec4 particle;

uniform mat4 viewProjection;
uniform float snowSize;
uniform vec2 displacement;
uniform vec3 minimum;
uniform vec3 extent;

out float edgeFade;

void main()
{
    vec3 offset = vec3(displacement.x, -displacement.y * particle.w, 0.0);

    // Loop the particle back to the top when it hits the bottom
    vec3 position = minimum + mod(particle.xyz - minimum + offset, extent);
    // Fade the particles near the edges, so they fade in at the top and out at the bottom 
    float height = position.y - minimum.y;
    edgeFade = smoothstep(0.0, 2.0, height)
        * smoothstep(0.0, 2.0, extent.y - height);
    gl_Position = viewProjection * vec4(position, 1.0);
    gl_PointSize = clamp(snowSize * particle.w * 12.0 / max(gl_Position.w, 1.0), 1.0, 16.0);
}
