#version 330 core
layout (location = 0) in vec3 position;
layout (location = 2) in mat4 model;
layout (location = 6) in vec3 instanceColor;

uniform mat4 pv;

out vec3 vColor;

void main()
{
    vColor = instanceColor;
    gl_Position = pv * model * vec4(position, 1.0f);
}
