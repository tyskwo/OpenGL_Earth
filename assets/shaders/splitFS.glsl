#version 330 core

					  in	  vec2		v_texCoord;		//the texture coordinate of the vertex
					  
					  uniform sampler2D texture_normal;	//the normal brightness texture

layout (location = 0) out	  vec4		o_color;		//normal color to pass to the first color attachment
layout (location = 1) out	  vec4		o_colorBright;	//bright color to pass to the second color attachment



void main()
{
//get the color of the fragment
	o_color = texture2D(texture_normal, vec2(v_texCoord[0], v_texCoord[1]));

//check whether the brightness is higher than threshold, if so output as bright color
    float brightness = dot(o_color.rgb, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > 1.0) 
	{
		o_colorBright = vec4(o_color.rgb, 1.0);
	}

//ekse output as black
	else
	{
		vec3 temp = vec3(0.0);
		o_colorBright = vec4(temp, 1.0);
	}
}