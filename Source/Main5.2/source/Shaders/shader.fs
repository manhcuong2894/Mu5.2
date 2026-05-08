#version 330 compatibility
out vec4 FragColor;

in vec2 TexCoord;
in vec4 to_light;
in vec3 FragPos;
in vec3 Normal;

uniform sampler2D texture1;
uniform vec3 lightPos = vec3(0.0, 1000.0, 1000.0);
uniform vec3 viewPos;
uniform float ambientStrength = 0.5;
uniform float specularStrength = 0.3;

void main() {
    float alpha = texture(texture1, TexCoord).a * to_light.a;
    if(alpha < 0.1) discard;

    vec4 textureColor = texture(texture1, TexCoord);
    
    // Ambient
    vec3 ambient = ambientStrength * to_light.rgb;
    
    // Diffuse
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * to_light.rgb;
    
    // Specular
    vec3 viewDir = normalize(viewPos - FragPos); 
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);
    
    vec3 result = (ambient + diffuse + specular) * textureColor.rgb;
    
    FragColor = vec4(result, alpha);
}
