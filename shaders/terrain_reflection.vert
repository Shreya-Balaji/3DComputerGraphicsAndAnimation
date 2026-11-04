#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 viewProjection;
uniform vec3 sunDir;
uniform vec3 sunColor;
uniform vec3 moonDir;
uniform vec3 moonColor;
uniform vec3 ambientColor;

out vec3 snowColor;

void main()
{
    vec3 N = normalize(normal);
    snowColor = vec3(0.9) * (ambientColor
        + sunColor * max(dot(N, normalize(-sunDir)), 0.0)
        + moonColor * max(dot(N, normalize(-moonDir)), 0.0));
    gl_Position = viewProjection * model * vec4(position, 1.0);
}
