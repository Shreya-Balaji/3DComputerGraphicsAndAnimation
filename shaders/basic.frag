#version 330 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 objectColor;

out vec4 FragColor;

void main()
{
    vec3 N = normalize(Normal);
    vec3 L = normalize(-lightDir);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);

    vec3 ambient = 0.22 * objectColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * objectColor;

    float spec = pow(max(dot(N, H), 0.0), 32.0);
    vec3 specular = vec3(0.35) * spec;

    vec3 color = ambient + diffuse + specular;

    FragColor = vec4(color, 1.0);
}
