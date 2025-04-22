#version 330 core

in vec3 vColor;
in vec2 vTexCoord;
flat in int vTexIndex;

uniform sampler2D atlasTexture;
uniform int atlasRows;
uniform int atlasColumns;

out vec4 FragColor;

/* gamma = 2.0f */
const float INV_GAMMA = 1.0f / 2.2f;

void main()
{
    if(vTexIndex == -1) {
        vec3 gammaCorrected = pow(vColor, vec3(INV_GAMMA));
        FragColor = vec4(gammaCorrected, 1.0f);
        return;
    } else {
        // calculate atlas tile UV
        int col = vTexIndex % atlasColumns;
        int row = vTexIndex / atlasColumns;
        vec2 tileSize = vec2(1.0 / atlasColumns, 1.0 / atlasRows);
        vec2 offset = vec2(col, row) * tileSize;
        vec2 atlasUV = offset + vec2(vTexCoord.x, 1.0 - vTexCoord.y) * tileSize;
        
        float alpha = texture(atlasTexture, atlasUV).r;

        float gamma = 2.2f;
        vec3 gammaCorrected = pow(vec3(alpha) * vColor, vec3(INV_GAMMA));
        FragColor = vec4(gammaCorrected, alpha);
    }

}
