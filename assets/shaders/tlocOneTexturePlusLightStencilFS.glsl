#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
uniform sampler2D s_texture;

layout (location = 0) out vec4 o_color;
layout (location = 1) out vec4 o_stencil;

void main()
{
	o_color = texture2D(s_texture, vec2(v_texCoord[0], 1.0 - v_texCoord[1]));
  o_stencil = o_color * 0.5;
}
