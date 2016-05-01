#version 330 core

in  	vec2 		v_texCoord;

uniform sampler2D 	s_texture;

out 	vec4 		o_color;

void main()
{
	o_color   = texture2D(s_texture, vec2(v_texCoord[0], 1.0 - v_texCoord[1]));
}
