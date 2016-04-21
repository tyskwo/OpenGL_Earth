#version 330 core

in vec2 v_texCoord;

out vec4 o_color;

uniform sampler2D texture_bright;
uniform float     weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{             
    vec2 tex_offset = 1.0 / textureSize(texture_bright, 0);				  //gets size of single texel
    vec3 result     = texture2D(texture_bright, v_texCoord).rgb * weight[0]; //current fragment's contribution

    for(int i = 1; i < 5; ++i)
    {
        result += texture2D(texture_bright, v_texCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        result += texture2D(texture_bright, v_texCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }

    for(int i = 1; i < 5; ++i)
    {
        result += texture2D(texture_bright, v_texCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        result += texture2D(texture_bright, v_texCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }

    o_color = vec4(result, 1.0);
}