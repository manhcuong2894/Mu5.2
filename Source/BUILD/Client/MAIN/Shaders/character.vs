#version 330 compatibility

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in float aPositionNode;
layout(location = 4) in float aNormalNode;

out vec2 TexCoord;
out vec4 VertexColor;

uniform mat4 uBones[200];
uniform vec3 uBodyOrigin;
uniform vec3 uBodyLight;
uniform vec3 uLightDir;
uniform vec2 uTexCoordOffset;
uniform float uBodyScale;
uniform float uAlpha;
uniform int uTranslate;
uniform int uUseLighting;
uniform int uRenderMode;
uniform float uWorldTime;

void main()
{
    int positionNode = clamp(int(aPositionNode + 0.5), 0, 199);
    int normalNode = clamp(int(aNormalNode + 0.5), 0, 199);

    vec3 worldPosition = (uBones[positionNode] * vec4(aPosition, 1.0)).xyz;
    vec3 worldNormal = mat3(uBones[normalNode]) * aNormal;

    if (uTranslate != 0)
    {
        worldPosition *= uBodyScale;
        worldPosition += uBodyOrigin;
    }

    vec3 color = uBodyLight;
    if (uUseLighting != 0)
    {
        float luminosity = dot(normalize(worldNormal), normalize(uLightDir)) * 0.8 + 0.4;
        luminosity = max(luminosity, 0.2);
        color *= luminosity;
    }

    float wave = mod(uWorldTime, 10000.0) * 0.0001;
    float wave2 = mod(uWorldTime, 5000.0) * 0.00024 - 0.4;
    vec3 lightVector = vec3(0.0, -0.1, -0.8);
    vec3 chromeL = vec3(cos(uWorldTime * 0.001), sin(uWorldTime * 0.002), 1.0);

    if (uRenderMode == 1)
    {
        TexCoord = vec2(worldNormal.z * 0.5 + wave, worldNormal.y * 0.5 + wave * 2.0);
    }
    else if (uRenderMode == 2)
    {
        TexCoord = vec2((worldNormal.z + worldNormal.x) * 0.8 + wave2 * 2.0,
                        (worldNormal.y + worldNormal.x) * 1.0 + wave2 * 3.0);
    }
    else if (uRenderMode == 3)
    {
        float d = dot(worldNormal, lightVector);
        TexCoord = vec2(d, 1.0 - d);
    }
    else if (uRenderMode == 4)
    {
        float d = dot(worldNormal, chromeL);
        TexCoord = vec2(d + worldNormal.y * 0.5 + chromeL.y * 3.0,
                        1.0 - d - (worldNormal.z * 0.5 + wave * 3.0)) + uTexCoordOffset;
    }
    else if (uRenderMode == 5)
    {
        float d = dot(worldNormal, chromeL);
        TexCoord = vec2(d + worldNormal.y * 3.0 + chromeL.y * 5.0,
                        1.0 - d - (worldNormal.z * 2.5 + wave));
    }
    else if (uRenderMode == 6)
    {
        float c = (worldNormal.z + worldNormal.x) * 0.8 + wave2 * 2.0;
        TexCoord = vec2(c, c);
    }
    else if (uRenderMode == 7)
    {
        float c = (worldNormal.z + worldNormal.x) * 0.8 + uWorldTime * 0.00006;
        TexCoord = vec2(c, c);
    }
    else if (uRenderMode == 8)
    {
        TexCoord = worldNormal.xy + uTexCoordOffset;
    }
    else if (uRenderMode == 9)
    {
        TexCoord = vec2(worldNormal.z * 0.5 + 0.2, worldNormal.y * 0.5 + 0.5);
    }
    else
    {
        TexCoord = aTexCoord + uTexCoordOffset;
    }

    VertexColor = vec4(color, uAlpha);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(worldPosition, 1.0);
}
