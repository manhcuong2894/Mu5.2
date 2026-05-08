#version 330 compatibility

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in vec2 vSize[];
in vec4 vColor[];
in float vRotation[];
in vec4 vTexRect[];

out vec2 TexCoord;
out vec4 VertexColor;

void EmitParticleVertex(vec4 viewCenter, vec2 offset, vec2 texCoord)
{
    vec4 viewPosition = viewCenter + vec4(offset.x, offset.y, 0.0, 0.0);
    gl_Position = gl_ProjectionMatrix * viewPosition;
    TexCoord = texCoord;
    VertexColor = vColor[0];
    EmitVertex();
}

void main()
{
    vec2 halfSize = vSize[0] * 0.5;
    float radiansValue = radians(vRotation[0]);
    float c = cos(radiansValue);
    float s = sin(radiansValue);
    mat2 rotation = mat2(c, s, -s, c);

    vec2 p0 = rotation * vec2(-halfSize.x, -halfSize.y);
    vec2 p1 = rotation * vec2( halfSize.x, -halfSize.y);
    vec2 p2 = rotation * vec2(-halfSize.x,  halfSize.y);
    vec2 p3 = rotation * vec2( halfSize.x,  halfSize.y);

    vec4 viewCenter = gl_ModelViewMatrix * gl_in[0].gl_Position;

    vec4 texRect = vTexRect[0];
    vec2 uv0 = texRect.xy;
    vec2 uv1 = texRect.xy + texRect.zw;

    EmitParticleVertex(viewCenter, p0, vec2(uv0.x, uv1.y));
    EmitParticleVertex(viewCenter, p1, vec2(uv1.x, uv1.y));
    EmitParticleVertex(viewCenter, p2, vec2(uv0.x, uv0.y));
    EmitParticleVertex(viewCenter, p3, vec2(uv1.x, uv0.y));
    EndPrimitive();
}
