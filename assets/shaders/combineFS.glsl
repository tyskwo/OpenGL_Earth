#version 330 core

in		vec2	  v_texCoord;

uniform sampler2D texture_normal;
uniform sampler2D texture_bright;

out		vec4	  o_color;


void main()
{             
    const float gamma = 2.2f;
	float exposure	  = 1.0f;

//get the color from each texture
    vec4 hdrColor   = texture2D(texture_normal, v_texCoord).rgba;      
    vec4 bloomColor = texture2D(texture_bright, v_texCoord).rgba;

//add the colors (additive blending)
    hdrColor += bloomColor;

//gamma correct
    vec4 result = vec4(1.0f) - exp(-hdrColor * exposure);
		 result = pow(result, vec4(1.0f / gamma));

//pass to the renderer
    o_color = result;
}  