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
uniform vec3 sunColor;
uniform vec3 moonDir;
uniform vec3 moonColor;
uniform vec3 ambientColor;
uniform samplerCube envMap; 
uniform float envMaxLod = 8.0;
uniform bool useEnvMap = false;
uniform sampler2D kdTexture;
uniform bool hasKdTexture;

uniform sampler2D normalTexture;
uniform bool hasNormalTexture;
uniform bool twoScaleSampling = false;

out vec4 FragColor;

void main()
{
    vec3 baseColor = kd;
    vec2 broadUV = TexCoord * 0.25 + vec2(0.37, 0.61);

    if (hasKdTexture) {
        baseColor = texture(kdTexture, TexCoord).rgb;
        if (twoScaleSampling)
            baseColor = mix(baseColor, texture(kdTexture, broadUV).rgb, 0.3);
    }

    vec3 N = normalize(Normal);

    if (hasNormalTexture) {
        vec3 tangentNormal =
            texture(normalTexture, TexCoord).rgb * 2.0 - 1.0;
        if (twoScaleSampling) {
            vec3 broadNormal = texture(normalTexture, broadUV).rgb * 2.0 - 1.0;
            tangentNormal = normalize(mix(tangentNormal, broadNormal, 0.3));
        }

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

    vec3 L2 = normalize(-moonDir);
    float diff2 = max(dot(N, L2), 0.0);
    vec3 moonDiffuse = diff2 * baseColor * moonColor;

    vec3 ambient = ambientColor * baseColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * baseColor * sunColor;

    float spec = 0.0;

    if (diff > 0.0) {
        spec = pow(
            max(dot(N, H), 0.0),
            max(shininess, 1.0)
        );
    }

    vec3 specular = ks * spec * sunColor;
    vec3 color = ambient + diffuse + specular + moonDiffuse;
    if(useEnvMap){
      vec3 R = reflect(-V, N);
      float roughness = sqrt(2.0 / (shininess + 2.0));
      float lod = roughness * envMaxLod;
      vec3 env = textureLod(envMap, R, lod).rgb; 
      color += ks * env; 
    }
    FragColor = vec4(color, transparency);
}
