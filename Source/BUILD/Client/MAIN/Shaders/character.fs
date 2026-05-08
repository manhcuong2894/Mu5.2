#version 330 compatibility

in vec2 vTexCoord;
in vec3 vNormalVS;
in vec3 vViewDirVS;
in vec4 vVertexColor;

uniform sampler2D texture1;
uniform vec3 uBodyLight = vec3(1.0, 1.0, 1.0);
uniform vec3 uEmissionColor = vec3(1.0, 0.85, 0.35);
uniform vec3 uChromeColor = vec3(0.90, 0.95, 1.0);
uniform vec3 uDebugTintColor = vec3(1.0, 0.25, 0.25);
uniform float uAlpha = 1.0;
uniform float uAmbientStrength = 0.45;
uniform float uSpecularStrength = 0.15;
uniform float uRimStrength = 0.10;
uniform float uEmissionStrength = 0.0;
uniform float uChromeStrength = 0.0;
uniform float uAlphaReference = 0.10;
uniform float uDebugTintStrength = 0.0;
uniform float uTime = 0.0;
uniform bool uUseLighting = true;
uniform bool uUseTexture = true;
uniform bool uUseAlphaTest = true;
uniform bool uUseSpecular = true;
uniform bool uUseRim = false;
uniform bool uUseEmission = false;
uniform bool uUseChrome = false;
uniform bool uPulseEmission = false;
uniform bool uUseDebugTint = false;

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
    float brightnessScale = clamp(0.74 + uAmbientStrength, 0.70, 0.88);

    if (!uUseLighting)
    {
        finalColor = baseColor.rgb * clamp((vertexLight * 0.82) + (uBodyLight * (0.05 + (uAmbientStrength * 0.25))), 0.0, 0.92);
        brightnessScale = clamp(0.72 + uAmbientStrength, 0.68, 0.86);
    }

    if (uUseSpecular)
    {
        vec3 halfDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfDir), 0.0), 16.0);
        finalColor += vec3(spec) * uSpecularStrength;
    }

    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 2.5);

    if (uUseRim)
    {
        finalColor += uBodyLight * fresnel * (uRimStrength * 0.35);
    }

    if (uUseChrome)
    {
        vec3 chrome = uChromeColor * (0.12 + (fresnel * 0.28));
        finalColor = mix(finalColor, max(finalColor, chrome), clamp(uChromeStrength, 0.0, 0.45));
    }

    if (uUseEmission)
    {
        float pulse = uPulseEmission ? (sin(uTime * 4.0) * 0.5 + 0.5) : 1.0;
        finalColor += uEmissionColor * uEmissionStrength * pulse;
    }

    finalColor *= brightnessScale;

    if (uUseDebugTint)
    {
        vec3 tinted = (finalColor * 0.55) + (uDebugTintColor * 0.45);
        finalColor = mix(finalColor, tinted, clamp(uDebugTintStrength, 0.0, 1.0));
    }

    FragColor = vec4(clamp(finalColor, 0.0, 1.0), outAlpha);
}
