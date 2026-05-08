#version 330 compatibility

layout(location = 0) in vec3 aCenter;
layout(location = 1) in vec2 aSize;
layout(location = 2) in vec4 aColor;
layout(location = 3) in float aRotation;

out vec2 vSize;
out vec4 vColor;
out float vRotation;

void main()
{
    gl_Position = vec4(aCenter, 1.0);
    vSize = aSize;
    vColor = aColor;
    vRotation = aRotation;
}
