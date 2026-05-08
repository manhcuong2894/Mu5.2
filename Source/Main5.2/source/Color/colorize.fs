// MU Season 6 - Colorization Shader (Fragment)
// Otimizado para máxima performance

#version 120

// Inputs
uniform sampler2D uTexture;
varying vec3 fragColor;
varying vec3 fragNormal;
varying vec2 fragTexCoord;
varying vec3 fragPos;

// Uniforms de iluminação
uniform vec3 uLightDir;
uniform vec3 uLightColor;
uniform vec3 uAmbient;

void main()
{
    // Amostrar textura
    vec4 texColor = texture2D(uTexture, fragTexCoord);
    
    // Se textura é transparente, descartar
    if (texColor.a < 0.5)
        discard;
    
    // Aplicar colorização com multiplicação
    // Isso mantém os detalhes da textura enquanto aplica a cor
    vec3 baseColor = texColor.rgb * fragColor;
    
    // Iluminação difusa simples (Lambertian)
    vec3 norm = normalize(fragNormal);
    float diff = max(dot(norm, uLightDir), 0.0);
    
    // Resultado: cor base * (luz ambiente + luz difusa)
    vec3 result = baseColor * (uAmbient + uLightColor * diff);
    
    // Output
    gl_FragColor = vec4(result, texColor.a);
}
