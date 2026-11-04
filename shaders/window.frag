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
uniform vec3 sunColor;
uniform vec3 moonDir;
uniform vec3 moonColor;
uniform vec3 ambientColor;
uniform samplerCube envMap;
uniform float envMaxLod = 8.0;
uniform bool useEnvMap = false;
uniform float windowOpacity;
uniform vec3 cabinLightPos;
uniform vec3 cabinLightColor;
uniform float cabinLightAmount;
uniform float cabinLightRadius;
uniform float cabinLightIntensity;

out vec4 FragColor;

vec3 sampleEnvironment(vec3 R, float lod)
{
    vec3 up = abs(R.y) < 0.999 ? vec3(0, 1, 0) : vec3(1, 0, 0);
    vec3 T = normalize(cross(up, R));
    vec3 B = cross(R, T);
    float texelSize = 2.0 * exp2(lod) / float(textureSize(envMap, 0).x);
    vec3 color = vec3(0.0);
    for (int y = -1; y <= 1; ++y) {
        for (int x = -1; x <= 1; ++x) {
            float weight = float((x == 0 ? 2 : 1) * (y == 0 ? 2 : 1));
            color += weight * textureLod(envMap, R + texelSize * (float(x) * T + float(y) * B), lod).rgb;
        }
    }
    return color / 16.0;
}

void main()
{
    vec3 baseColor = kd;
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 Lsun = normalize(-lightDir);
    vec3 Lmoon = normalize(-moonDir);
    vec3 H = normalize(Lsun + V);

    float diff = max(dot(N, Lsun), 0.0);
    float spec = 0.0;
    if (diff > 0.0)
        spec = pow(max(dot(N, H), 0.0), max(shininess, 1.0));

    vec3 ambient = ambientColor * baseColor;
    vec3 diffuse = diff * baseColor * sunColor;
    vec3 specular = ks * spec * sunColor;
    vec3 moonDiffuse = max(dot(N, Lmoon), 0.0) * baseColor * moonColor;
    vec3 color = ambient + diffuse + specular + moonDiffuse;
    if (useEnvMap) {
        vec3 R = reflect(-V, N);
        float lod = sqrt(2.0 / (shininess + 2.0)) * envMaxLod;
        color += ks * sampleEnvironment(R, lod);
    }

    vec3 toLamp = cabinLightPos - FragPos;
    float d = length(toLamp);
    vec3 Lc = toLamp / d;
    float falloff = clamp(1.0 - d / cabinLightRadius, 0.0, 1.0);
    falloff = pow(falloff, 2);
    float lamp = max(dot(N, Lc), 0.0) * falloff * cabinLightAmount * cabinLightIntensity;
    color += lamp * baseColor * cabinLightColor;
    if (shininess > 300.0)
        color += cabinLightColor * cabinLightAmount * 0.3;
    FragColor = vec4(color, transparency * windowOpacity);
}
