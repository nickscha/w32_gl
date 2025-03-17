#version 330 core
out vec4 FragColor;

uniform vec3 color;

float near = 0.1f; 
float far  = 100.0f; 
  
float linear_depth(float depth) 
{
    float z = depth * 2.0f - 1.0f; // back to NDC 
    return (2.0f * near * far) / (far + near - z * (far - near));	
}

void main()
{
    //FragColor = vec4(color, 1.0f);
    // https://learnopengl.com/Advanced-Lighting/Gamma-Correction
    // apply gamma correction 2.2
    float gamma = 2.2f;
    FragColor = vec4(pow(color, vec3(1.0f/gamma)), 1.0f);

    //float depth = linear_depth(gl_FragCoord.z) / far; // divide by far for demonstration
    //FragColor = vec4(vec3(depth), 1.0);
}
