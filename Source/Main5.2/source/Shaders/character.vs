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

    TexCoord = aTexCoord + uTexCoordOffset;
    VertexColor = vec4(color, uAlpha);
    gl_Position = gl_ModelViewProjectionMatrix * vec4(worldPosition, 1.0);
}
