#version 330 compatibility

in vec2 vTexCoord;
in vec3 vNormalVS;
in vec3 vViewDirVS;
in vec4 vVertexColor;

uniform sampler2D texture1;
uniform vec3 uBodyLight = vec3(1.0, 1.0, 1.0);
uniform vec3 uEmissionColor = vec3(1.0, 0.85, 0.35);
uniform float uAlpha = 1.0;
uniform float uAmbientStrength = 0.45;
uniform float uSpecularStrength = 0.10;
uniform float uEmissionStrength = 0.45;
uniform float uAlphaReference = 0.10;
uniform float uTime = 0.0;
uniform bool uUseLighting = true;
uniform bool uUseTexture = true;
uniform bool uUseAlphaTest = true;
uniform bool uUseSpecular = true;
uniform bool uPulseEmission = true;

out vec4 FragColor;

void main()
{
    vec4 baseColor = uUseTexture ? texture(texture1, vTexCoord) : vec4(1.0);
    float outAlpha = baseColor.a * vVertexColor.a * uAlpha;

    if (uUseAlphaTest && outAlpha < uAlphaReference)
    {
        discard;
    }

    vec3 normal = normalize(vNormalVS);
    vec3 viewDir = normalize(vViewDirVS);
    vec3 lightDir = normalize(vec3(0.20, -0.45, 0.85));
    vec3 vertexLight = clamp(vVertexColor.rgb, 0.0, 1.0);
    vec3 finalColor = baseColor.rgb * vertexLight;

    if (uUseSpecular)
    {
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfDir), 0.0), 14.0);
        finalColor += vec3(spec) * uSpecularStrength;
    }

    float pulse = uPulseEmission ? (sin(uTime * 4.0) * 0.5 + 0.5) : 1.0;
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.0);

    finalColor += uEmissionColor * (uEmissionStrength * (0.35 + pulse * 0.25));
    finalColor += uBodyLight * fresnel * 0.05;

    FragColor = vec4(clamp(finalColor, 0.0, 1.0), outAlpha);
}
