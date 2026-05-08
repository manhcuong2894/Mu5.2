#version 330 compatibility

in vec2 vTexCoord;
in vec3 vNormalVS;
in vec3 vViewDirVS;
in vec4 vVertexColor;

uniform sampler2D texture1;
uniform vec3 uBodyLight = vec3(1.0, 1.0, 1.0);
uniform vec3 uDebugTintColor = vec3(1.0, 0.25, 0.25);
uniform float uAlpha = 1.0;
uniform float uAmbientStrength = 0.45;
uniform float uAlphaReference = 0.10;
uniform float uDebugTintStrength = 0.0;
uniform bool uUseLighting = true;
uniform bool uUseTexture = true;
uniform bool uUseAlphaTest = true;
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

    vec3 vertexLight = clamp(vVertexColor.rgb, 0.0, 1.0);
    vec3 lighting = vertexLight;
    float brightnessScale = clamp(0.72 + uAmbientStrength, 0.68, 0.84);

    if (!uUseLighting)
    {
        lighting = clamp((vertexLight * 0.80) + (uBodyLight * (0.04 + (uAmbientStrength * 0.25))), 0.0, 0.88);
        brightnessScale = clamp(0.70 + uAmbientStrength, 0.66, 0.82);
    }

    vec3 shaded = baseColor.rgb * lighting;
    shaded *= brightnessScale;

    if (uUseDebugTint)
    {
        vec3 tinted = (shaded * 0.55) + (uDebugTintColor * 0.45);
        shaded = mix(shaded, tinted, clamp(uDebugTintStrength, 0.0, 1.0));
    }

    FragColor = vec4(clamp(shaded, 0.0, 1.0), outAlpha);
}
