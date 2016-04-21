#version 330 core

in vec2 v_texCoord;

out vec4 o_color;

uniform sampler2D texture_normal;
uniform sampler2D texture_bright;
//uniform float	  exposure;

void main()
{             
    const float gamma = 2.2;
	float exposure = 0.5f;

    vec4 hdrColor   = texture2D(texture_normal, v_texCoord).rgba;      
    vec4 bloomColor = texture2D(texture_bright, v_texCoord).rgba;
    hdrColor += bloomColor; // additive blending

    // tone mapping
    vec4 result = vec4(1.0) - exp(-hdrColor * exposure);
		 result = pow(result, vec4(1.0 / gamma));

    o_color = result;
}  