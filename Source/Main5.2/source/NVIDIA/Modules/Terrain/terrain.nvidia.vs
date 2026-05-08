#version 450 core

// Input
layout(location = 0) in vec3 Position;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 TexCoord;
layout(location = 3) in vec3 Tangent;
layout(location = 4) in vec3 Bitangent;

// Uniforms
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;
uniform mat4 uNormalMatrix;

// Output
out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
    mat3 TBN;
    flat int LODLevel;
} vs_out;

void main()
{
    // Calcular posição do fragmento em world space
    vs_out.FragPos = vec3(uModel * vec4(Position, 1.0));
    
    // Transformar normal
    vs_out.Normal = normalize(vec3(uNormalMatrix * vec4(Normal, 0.0)));
    
    // Passar coordenadas de textura
    vs_out.TexCoord = TexCoord;
    
    // Calcular matriz TBN (Tangent-Bitangent-Normal) para normal mapping
    vec3 T = normalize(vec3(uNormalMatrix * vec4(Tangent, 0.0)));
    vec3 B = normalize(vec3(uNormalMatrix * vec4(Bitangent, 0.0)));
    vec3 N = vs_out.Normal;
    vs_out.TBN = mat3(T, B, N);
    
    // LOD Level baseado na distância
    float distToCamera = length((uView * uModel * vec4(Position, 1.0)).xyz);
    if (distToCamera < 300.0)
        vs_out.LODLevel = 0;  // Alta qualidade
    else if (distToCamera < 500.0)
        vs_out.LODLevel = 1;  // Média qualidade
    else
        vs_out.LODLevel = 2;  // Baixa qualidade
    
    // Posição final
    gl_Position = uProj * uView * uModel * vec4(Position, 1.0);
}
