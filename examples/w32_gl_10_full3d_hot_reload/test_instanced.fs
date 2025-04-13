#version 330 core

in vec3 vColor;
out vec4 FragColor;

void main()
{
    // apply gamma correction 2.2
    float gamma = 2.2f;
    vec3 gammaCorrected = pow(vColor, vec3(1.0f / gamma));
    FragColor = vec4(gammaCorrected, 1.0f);
}
