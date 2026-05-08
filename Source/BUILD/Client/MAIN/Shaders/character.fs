#version 330 compatibility

in vec2 TexCoord;
in vec4 VertexColor;

out vec4 FragColor;

uniform sampler2D texture1;
uniform int uRenderMode;

void main()
{
    if (uRenderMode == 10)
    {
        FragColor = vec4(VertexColor.rgb * 0.55, VertexColor.a * 0.75);
        return;
    }

    vec4 textureColor = texture(texture1, TexCoord);
    float alpha = textureColor.a * VertexColor.a;

    if (alpha < 0.1)
    {
        discard;
    }

    FragColor = vec4(textureColor.rgb * VertexColor.rgb, alpha);
}
