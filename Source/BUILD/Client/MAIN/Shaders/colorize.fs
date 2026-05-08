#version 330 compatibility

in vec2 vTexCoord;
in vec3 vNormalVS;
in vec4 vVertexColor;

uniform sampler2D texture1;
uniform int uColorMode = 0;
uniform int uColorValue = 0;
uniform vec3 uLightDir = vec3(-0.30, 0.60, 0.80);
uniform vec3 uLightColor = vec3(1.0, 1.0, 1.0);

out vec4 FragColor;

vec3 GetTintColor(int mode, int value)
{
    if (mode == 1)
    {
        vec3 classPalette[6] = vec3[6](
            vec3(0.95, 0.35, 0.35),
            vec3(0.35, 0.55, 1.00),
            vec3(0.75, 0.35, 0.95),
            vec3(0.35, 0.95, 0.55),
            vec3(1.00, 0.80, 0.25),
            vec3(0.95, 0.55, 0.20)
        );
        return classPalette[value % 6];
    }

    if (mode == 2)
    {
        vec3 gradePalette[5] = vec3[5](
            vec3(1.00, 1.00, 1.00),
            vec3(0.45, 0.95, 0.55),
            vec3(0.35, 0.70, 1.00),
            vec3(0.90, 0.45, 1.00),
            vec3(1.00, 0.75, 0.25)
        );
        return gradePalette[value % 5];
    }

    if (mode == 3)
    {
        vec3 customPalette[6] = vec3[6](
            vec3(1.00, 0.30, 0.30),
            vec3(0.30, 1.00, 0.30),
            vec3(0.30, 0.55, 1.00),
            vec3(1.00, 0.85, 0.30),
            vec3(1.00, 0.35, 0.85),
            vec3(0.30, 1.00, 0.95)
        );
        return customPalette[value % 6];
    }

    return vec3(1.0, 1.0, 1.0);
}

void main()
{
    vec4 baseColor = texture(texture1, vTexCoord);
    if (baseColor.a * vVertexColor.a < 0.10)
    {
        discard;
    }

    vec3 normal = normalize(vNormalVS);
    float ndotl = max(dot(normal, normalize(uLightDir)), 0.25);
    vec3 tint = GetTintColor(uColorMode, uColorValue);

    vec3 finalColor = baseColor.rgb * vVertexColor.rgb;
    finalColor *= mix(vec3(1.0), tint, 0.35);
    finalColor *= uLightColor * ndotl;

    FragColor = vec4(finalColor, baseColor.a * vVertexColor.a);
}
