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
uniform sampler2D kdTexture;
uniform bool hasKdTexture;
uniform vec3 cabinLightPos;
uniform vec3 cabinLightColor;
uniform float cabinLightAmount;
uniform float cabinLightRadius;
uniform float cabinLightIntensity;
uniform sampler2D normalTexture;
uniform bool hasNormalTexture;
uniform bool twoScaleSampling = false;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform bool useShadows = false;
uniform vec3 shadowToLight = vec3(0.0, 1.0, 0.0);
out vec4 FragColor;

float shadowFactor(vec3 fragPos, vec3 N, vec3 L)
{
    vec4 clip = lightSpaceMatrix * vec4(fragPos, 1.0);
    vec3 p = clip.xyz / clip.w * 0.5 + 0.5;

    if (p.z > 1.0 || p.x < 0.0 || p.x > 1.0 || p.y < 0.0 || p.y > 1.0)
        return 1.0;

    float bias = max(0.002 * (1.0 - dot(N, L)), 0.0005);

    vec2 texel = 1.0 / vec2(textureSize(shadowMap, 0));
    float inShadow = 0.0;
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float closest = texture(shadowMap, p.xy + vec2(x, y) * texel).r;
            inShadow += (p.z - bias > closest) ? 1.0 : 0.0;
        }
    }
    float edge = min(min(p.x, 1.0 - p.x), min(p.y, 1.0 - p.y));
    float fade = smoothstep(0.0, 0.1, edge);
    return mix(1.0, 1.0 - inShadow / 9.0, fade);
}
const vec2  ropeXY = vec2(-5.4, 0.92);
const float ropeThickness= 0.025; 
const float ropeHalf = 96.0;
float ropeShadow(vec3 fragPos, vec3 toLight)
{
    vec2 p = fragPos.xy;                  
    vec2 l = toLight.xy;
    float ll = dot(l, l);
    if (ll < 1e-6){
        return 1.0; 
    }           
    float t = dot(ropeXY - p, l) / ll;   
    if (t <= 0.0){
        return 1.0;
    }            
    float zHit = fragPos.z + t * toLight.z;
    if (abs(zHit) > ropeHalf){
        return 1.0;   
    }
    float d = length(p + t * l - ropeXY);         
    return smoothstep(0.5 * ropeThickness, ropeThickness, d);
}
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
    vec3 ambient = ambientColor * baseColor;
    float diff = max(dot(N, L), 0.0);

    bool sunOrMoon = lightDir.y<0.0;
    vec3 Ls = sunOrMoon ? L : L2;
    float shadow = 1.0;
    if (useShadows){
        shadow = min(shadowFactor(FragPos, N, Ls), ropeShadow(FragPos, shadowToLight));
    }
    vec3 diffuse  = diff * baseColor * sunColor * shadow;
    vec3 moonDiffuse = diff2 * baseColor * moonColor * shadow;

    float spec = 0.0;

    if (diff > 0.0) {
        spec = pow(
            max(dot(N, H), 0.0),
            max(shininess, 1.0)
        );
    }

    vec3 specular = ks * spec * sunColor * shadow;
    vec3 color = ambient + diffuse + specular + moonDiffuse;
    vec3 toLamp = cabinLightPos - FragPos;
    float d = length(toLamp);
    vec3 Lc = toLamp / d;
    float falloff = clamp(1.0 - d / cabinLightRadius, 0.0, 1.0);
    falloff = pow(falloff,2);
    float lamp = max(dot(N, Lc), 0.0) * falloff * cabinLightAmount * cabinLightIntensity;
    color += lamp * baseColor * cabinLightColor;
    FragColor = vec4(color, transparency);
}
