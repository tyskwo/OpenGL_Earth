#version 330 core

in		vec2	  v_texCoord;		//the vertex texture coordinate

uniform sampler2D texture_bright;	//the high pass brightness texture
uniform float     weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);	//weights to use for the blur

out		vec4	  o_color;			//color to pass to the renderer

void main()
{             
//get the size of a single texel
    vec2 tex_offset = 1.0 / textureSize(texture_bright, 0);
//the current fragment's weight of the blur
    vec3 result = texture2D(texture_bright, v_texCoord).rgb * weight[0];

//horizontal blur (5 pixels)
    for(int i = 1; i < 5; ++i)
    {
        result += texture2D(texture_bright, v_texCoord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        result += texture2D(texture_bright, v_texCoord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }

//vertical blur (5 pixels)
    for(int i = 1; i < 5; ++i)
    {
        result += texture2D(texture_bright, v_texCoord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        result += texture2D(texture_bright, v_texCoord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }

//pass the color to the renderer
    o_color = vec4(result, 1.0);
}