// MU Season 6 - Colorization Shader (Vertex)
// Otimizado para máxima performance - sem consumo extra

#version 120

// Inputs
attribute vec3 aPosition;
attribute vec3 aNormal;
attribute vec2 aTexCoord;
attribute vec3 aColor;        // Cor vertex original

// Uniforms
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

// Modo de colorização
uniform int uColorMode;       // 0=None, 1=ItemGrade, 2=Class, 3=Custom, 4=Elemental, 5=Team
uniform int uColorValue;      // Valor para lookup

// Paleta de cores (256 cores max, cada cor = 3 floats RGB)
uniform sampler1D uColorPalette;

// Outputs
varying vec3 fragColor;
varying vec3 fragNormal;
varying vec2 fragTexCoord;
varying vec3 fragPos;

void main()
{
    // Aplicar colorização apenas se ativada
    if (uColorMode > 0)
    {
        // Normalizar índice de cor para 0-1 (256 cores = 1/256)
        float colorIndex = float(uColorValue) / 255.0;
        
        // Lookup cor da paleta (lookup table é muito rápido)
        vec3 paletteColor = texture1D(uColorPalette, colorIndex).rgb;
        
        // Multiplicar cor original pela cor da paleta
        // Isso preserva sombras e detalhes enquanto aplica tint
        fragColor = aColor * paletteColor;
    }
    else
    {
        // Sem colorização, usar cor original
        fragColor = aColor;
    }

    // Passar dados para fragment shader
    fragNormal = normalize(mat3(uModel) * aNormal);
    fragTexCoord = aTexCoord;
    fragPos = vec3(uModel * vec4(aPosition, 1.0));

    // Posição final
    gl_Position = uProj * uView * uModel * vec4(aPosition, 1.0);
}
