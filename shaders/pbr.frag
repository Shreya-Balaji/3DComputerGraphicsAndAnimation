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
uniform float metallic;
uniform float roughness;
uniform vec3 lightDir;
uniform vec3 viewPos;
uniform vec3 sunColor;
uniform vec3 moonDir;
uniform vec3 moonColor;
uniform vec3 ambientColor;
uniform samplerCube envMap; 
uniform float envMaxLod = 8.0;
uniform bool useEnvMap;
uniform sampler2D kdTexture;
uniform bool hasKdTexture;

uniform sampler2D normalTexture;
uniform bool hasNormalTexture;
uniform bool twoScaleSampling = false;

out vec4 FragColor;
const float PI = 3.14159265;

float distributionGGX(float dotProductNH, float roughness){
    float a = roughness * roughness;
    float a2 = a*a;
    float denominator = dotProductNH * dotProductNH * (a2-1.0) + 1.0;
    float result = a2/(PI*denominator*denominator);
    return result;
}

float geometrySchlick(float dotProductNV, float roughness){
    float k = pow(roughness + 1.0, 2.0)/8.0;
    float result = (dotProductNV)/(dotProductNV * (1.0-k) +k);
    return result;
}
vec3 fresnelSchlick(float dotProductHV, vec3 F0){
    dotProductHV = clamp(dotProductHV,0.0,1.0);
    vec3 result = F0 + (1.0-F0) * pow(1.0 - dotProductHV, 5.0);
    return result;
}
vec3 shadeLight(vec3 N, vec3 V, vec3 L, vec3 lightColor, vec3 albedo){
    vec3 H = normalize(L + V);
    float dotProductHV = max(dot(H, V), 0.00000001);
    float dotProductNH = max(dot(N, H), 0.00000001);
    float dotProductNV = max(dot(N, V), 0.00000001);
    float dotProductNL = max(dot(N, L), 0.00000001);
    vec3 F0 = mix(vec3(0.04), albedo, metallic);
    float D = distributionGGX(dotProductNH, roughness);
    float G = geometrySchlick(dotProductNL, roughness) * geometrySchlick(dotProductNV, roughness);
    vec3 F = fresnelSchlick(dotProductHV, F0);
    vec3 specular = D * G * F / (4.0 * dotProductNV * max(dotProductNL, 0.001));
    vec3 kD = (1.0 - F) * (1.0 - metallic);
    vec3 diffuse = kD * albedo / PI;
    vec3 result = (diffuse + specular) * lightColor* dotProductNL;
    return result;


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
    vec3 Lsun= normalize(-lightDir);
    vec3 Lmoon = normalize(-moonDir);
    vec3 V = normalize(viewPos - FragPos);
    float NdotV = max(dot(N, V), 0.0001);
    vec3  F0 = mix(vec3(0.04), baseColor, metallic);
    bool  isGlass = shininess > 300.0;
    float rough   = isGlass ? 0.05 : roughness;
    vec3 ambient = ambientColor * baseColor; 
    vec3 env = vec3(0.0);
    if(useEnvMap){
        vec3 R   = reflect(-V, N);
        env = textureLod(envMap, R, rough * envMaxLod).rgb;
        vec3 Fenv = F0 + (max(vec3(1.0 - rough), F0) - F0) * pow(1.0 - NdotV, 5.0);
        vec3 kDenv = (1.0 - Fenv) * (1.0 - metallic);
        ambient = ambientColor * baseColor * kDenv + env * Fenv;
    }
    vec3 color = ambient + shadeLight(N,V,Lsun,sunColor,baseColor) + shadeLight(N,V,Lmoon,moonColor,baseColor);
    if (useEnvMap && isGlass) {
        float glassMix = 0.35 + 0.65 * pow(1.0 - NdotV, 3.0);
        color = mix(color, env, glassMix);
    }
    FragColor = vec4(color, transparency);
}
