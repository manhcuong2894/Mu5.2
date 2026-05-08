#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec4 to_light;
in vec3 FragPos;

uniform sampler2D texture1;
uniform float time;           // Tempo do jogo para animação
uniform float glowIntensity;  // Intensidade do brilho (0.0 a 1.0)
uniform vec3 glowColor;       // Cor do brilho (RGB)

void main() {
    vec4 textureColor = texture(texture1, TexCoord);
    
    // Shader Optimization: Discard fully transparent pixels early
    if(textureColor.a < 0.05) discard;

    vec4 lightColor = to_light;

    float maxIntensity = 1.5; // Permite brilho extra
    lightColor.rgb = min(lightColor.rgb, vec3(maxIntensity));

    FragColor = textureColor * lightColor;
    
    // Efeito de pulsação
    float pulse = sin(time * 3.0) * 0.5 + 0.5; // Oscila entre 0.0 e 1.0
    
    // Adicionar brilho pulsante
    vec3 glow = glowColor * glowIntensity * pulse;
    FragColor.rgb += glow;
    
    // Aumentar brilho nas áreas já claras (bloom effect)
    float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > 0.8) {
        FragColor.rgb *= 1.2;
    }
}
