
#version 330 core

layout(std140) uniform Material {
    vec3 kd;
    vec3 ks;
    float shininess;
    float transparency;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;

uniform vec3 lightDir;
uniform vec3 viewPos;

uniform sampler2D kdTexture;
uniform bool hasKdTexture;

uniform sampler2D normalTexture;
uniform bool hasNormalTexture;

out vec4 FragColor;

void main()
{
    vec3 baseColor = kd;

    if (hasKdTexture) {
        baseColor = texture(kdTexture, TexCoord).rgb;
    }

    vec3 N = normalize(Normal);

    if (hasNormalTexture) {
        vec3 tangentNormal =
            texture(normalTexture, TexCoord).rgb * 2.0 - 1.0;

        mat3 TBN = mat3(
            normalize(Tangent),
            normalize(Bitangent),
            normalize(Normal)
        );

        N = normalize(TBN * tangentNormal);
    }

    vec3 L = normalize(-lightDir);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(L + V);

    vec3 ambient = 0.30 * baseColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * baseColor;

    float spec = 0.0;

    if (diff > 0.0) {
        spec = pow(
            max(dot(N, H), 0.0),
            max(shininess, 1.0)
        );
    }

    vec3 specular = ks * spec;
    vec3 color = ambient + diffuse + specular;

    FragColor = vec4(color, transparency);
}
