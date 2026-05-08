#version 330 compatibility

layout (location = 0) in vec3 aPos;     // Position
layout (location = 1) in vec2 aTexCoord; // TexCoord
layout (location = 2) in vec4 aColor;    // Color (Attribute 2)
// layout (location = 3) in vec3 aNormal; // Normal - If we want to use Normals, we need to add them to ZzzBMD.cpp

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;
out vec4 to_light;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    FragPos = vec3(model * vec4(aPos, 1.0));
    // Normal = mat3(transpose(inverse(model))) * aNormal; // Need normal setup
    Normal = vec3(0.0, 1.0, 0.0); // Dummy normal for now until added in CPP
    TexCoord = aTexCoord;
    to_light = aColor; 
}
