#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoord;
layout (location = 2) in mat4 model;
layout (location = 6) in vec3 instanceColor;
layout (location = 9) in int textureIndex;

uniform mat4 pv;

out vec3 vColor;
out vec2 vTexCoord;
flat out int vTexIndex;

void main()
{
    vColor = instanceColor;
    vTexCoord = texCoord;
    vTexIndex = textureIndex;
    gl_Position = pv * model * vec4(position, 1.0f);
}
