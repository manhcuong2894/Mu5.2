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
uniform float uWave;
uniform float uWave2;
uniform float uWorldTime;
uniform vec3 uChromeL;
uniform int uTranslate;
uniform int uUseLighting;
uniform int uRenderMode;

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

    vec3 normal = worldNormal;
    vec2 chromeCoord = vec2(normal.z * 0.5 + uWave, normal.y * 0.5 + uWave * 2.0);

    if (uRenderMode == 2)
    {
        chromeCoord = vec2((normal.z + normal.x) * 0.8 + uWave2 * 2.0,
            (normal.y + normal.x) * 1.0 + uWave2 * 3.0);
    }
    else if (uRenderMode == 3)
    {
        float d = dot(normal, vec3(0.0, -0.1, -0.8));
        chromeCoord = vec2(d, 1.0 - d);
    }
    else if (uRenderMode == 4)
    {
        float d = dot(normal, uChromeL);
        chromeCoord = vec2(d + normal.y * 0.5 + uChromeL.y * 3.0,
            1.0 - d - (normal.z * 0.5 + uWave * 3.0));
    }
    else if (uRenderMode == 5)
    {
        float d = dot(normal, uChromeL);
        chromeCoord = vec2(d + normal.y * 3.0 + uChromeL.y * 5.0,
            1.0 - d - (normal.z * 2.5 + uWave));
    }
    else if (uRenderMode == 6)
    {
        float c = (normal.z + normal.x) * 0.8 + uWave2 * 2.0;
        chromeCoord = vec2(c, c);
    }
    else if (uRenderMode == 7)
    {
        float c = (normal.z + normal.x) * 0.8 + uWorldTime * 0.00006;
        chromeCoord = vec2(c, c);
    }
    else if (uRenderMode == 8 || uRenderMode == 9)
    {
        chromeCoord = normal.xy;
    }
    else if (uRenderMode == 10)
    {
        chromeCoord = vec2(normal.z * 0.5 + 0.2, normal.y * 0.5 + 0.5);
    }

    if (uRenderMode == 0)
    {
        TexCoord = aTexCoord + uTexCoordOffset;
    }
    else if (uRenderMode == 11)
    {
        TexCoord = aTexCoord;
    }
    else if (uRenderMode == 4 || uRenderMode == 8)
    {
        TexCoord = chromeCoord + uTexCoordOffset;
    }
    else if (uRenderMode == 9)
    {
        TexCoord = chromeCoord * aTexCoord + uTexCoordOffset;
    }
    else
    {
        TexCoord = chromeCoord;
    }

    VertexColor = vec4(color, uAlpha);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(worldPosition, 1.0);
}
