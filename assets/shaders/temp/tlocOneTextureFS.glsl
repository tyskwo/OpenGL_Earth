#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
out vec4 o_color;
uniform sampler2D s_texture;

void main()
{
	o_color = texture2D(s_texture, vec2(v_texCoord[0], 1.0 - v_texCoord[1]));
}
