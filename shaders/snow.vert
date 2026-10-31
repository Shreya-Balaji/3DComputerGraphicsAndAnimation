#version 330 core

layout(location = 0) in vec4 particle;

uniform mat4 viewProjection;
uniform float snowSize;
uniform vec2 displacement;
uniform vec3 minimum;
uniform vec3 extent;

void main()
{
    vec3 offset = vec3(displacement.x, -displacement.y * particle.w, 0.0);
    // Recycle flakes at the opposite edge without changing their initial vertex data.
    vec3 position = minimum + mod(particle.xyz - minimum + offset, extent);
    gl_Position = viewProjection * vec4(position, 1.0);
    gl_PointSize = clamp(snowSize * particle.w * 12.0 / max(gl_Position.w, 1.0), 1.0, 16.0);
}
