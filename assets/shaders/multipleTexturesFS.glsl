#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
out vec3 o_color;
uniform sampler2D s_texture;
uniform sampler2D s_texture_2;

void main()
{
  // NOTE: Tex co-ords flipped in 't'
	vec3 color_1 = texture2D(s_texture, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;

	vec3 color_2 = texture2D(s_texture_2, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;

	vec3 col1Inv = vec3(1,1,1) - color_1;

	o_color = color_2;
	o_color = (o_color * col1Inv) + color_1;
}
