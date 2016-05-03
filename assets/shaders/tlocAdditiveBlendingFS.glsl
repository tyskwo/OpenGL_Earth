#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
out vec4 o_color;

uniform sampler2D s_texture;
uniform sampler2D s_texture2;

void main()
{
	o_color = texture2D(s_texture, v_texCoord);
	o_color = o_color + texture2D(s_texture2, v_texCoord);
}
