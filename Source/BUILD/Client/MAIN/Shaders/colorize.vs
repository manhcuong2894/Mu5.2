#version 330 compatibility

out vec2 vTexCoord;
out vec3 vNormalVS;
out vec4 vVertexColor;

void main()
{
    vec4 viewPosition = gl_ModelViewMatrix * gl_Vertex;

    vTexCoord = gl_MultiTexCoord0.xy;
    vNormalVS = normalize(gl_NormalMatrix * gl_Normal);
    vVertexColor = gl_Color;

    gl_Position = gl_ProjectionMatrix * viewPosition;
}
