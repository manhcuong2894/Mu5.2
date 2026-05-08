#version 450 core

// Input
in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    mat3 TBN;
    flat int LODLevel;
} fs_in;

// Uniforms
uniform sampler2DArray AlbedoTextures;
uniform sampler2DArray NormalTextures;
uniform sampler2DArray RoughnessTextures;
uniform sampler2DArray MetallicTextures;

uniform vec3 uLightDir;
uniform vec3 uCameraPos;
uniform vec4 uLightColor;

// Output
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 NormalOutput;  // Para deferred rendering
layout(location = 2) out vec4 AlbedoOutput;  // Para deferred rendering

// PBR Functions
const float PI = 3.14159265359;

// Calcular distribuição normal (GGX/Trowbridge-Reitz)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// Calcular geometria (Schlick)
float GeometrySchlick(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlick(NdotV, roughness);
    float ggx1 = GeometrySchlick(NdotL, roughness);

    return ggx1 * ggx2;
}

// Calcular Fresnel (Schlick)
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // Selecionar textura baseado no LOD
    int texLayer = fs_in.LODLevel;

    // Carregar texturas
    vec3 albedo = texture(AlbedoTextures, vec3(fs_in.TexCoord, texLayer)).rgb;
    vec3 normal = texture(NormalTextures, vec3(fs_in.TexCoord, texLayer)).rgb;
    float roughness = texture(RoughnessTextures, vec3(fs_in.TexCoord, texLayer)).r;
    float metallic = texture(MetallicTextures, vec3(fs_in.TexCoord, texLayer)).r;

    // Transformar normal map
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(fs_in.TBN * normal);

    // Vetor de visão
    vec3 V = normalize(uCameraPos - fs_in.FragPos);
    vec3 L = normalize(uLightDir);
    vec3 H = normalize(V + L);

    // Calcular iluminação PBR
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    float distance = length(uLightDir);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = uLightColor.rgb * attenuation;

    // Cook-Torrance BRDF
    float D = DistributionGGX(normal, H, roughness);
    float G = GeometrySmith(normal, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = D * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(normal, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // Ambient (simplificado)
    vec3 ambient = albedo * 0.03;
    vec3 color = ambient + Lo;

    // Tone mapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    // Output para renderização forward
    FragColor = vec4(color, 1.0);

    // Output para deferred rendering
    NormalOutput = vec4(normal, 1.0);
    AlbedoOutput = vec4(albedo, metallic);
}
