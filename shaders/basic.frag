#version 330 core

layout(std140) uniform Material {
    vec3 kd;
    vec3 ks;
    float shininess;
    float transparency;
};

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);

    vec3 ambient = 0.30 * kd;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * kd;

    float spec = 0.0;
    if (diff > 0.0) {
        spec = pow(max(dot(N, H), 0.0), max(shininess, 1.0));
     }

     vec3 specular = ks * spec;
     vec3 color = ambient + diffuse + specular;

     FragColor = vec4(color, transparency);
}
