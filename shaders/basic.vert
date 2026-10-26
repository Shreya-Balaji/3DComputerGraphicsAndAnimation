#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec4 aTangent;

uniform mat4 model;
uniform mat4 viewProjection;

out vec3 FragPos;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;
out vec2 TexCoord;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);

    FragPos = worldPos.xyz;
    TexCoord = aTexCoord;

    // Transform the normal correctly, including non-uniform scaling.
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    vec3 N = normalize(normalMatrix * aNormal);

    // Transform the tangent and make it perpendicular to the normal.
    vec3 T = normalize(mat3(model) * aTangent.xyz);
    T = normalize(T - dot(T, N) * N);

    // Preserve the tangent-space handedness stored in aTangent.w.
    vec3 B = normalize(cross(N, T)) * aTangent.w;

    Normal = N;
    Tangent = T;
    Bitangent = B;

    gl_Position = viewProjection * worldPos;
}
