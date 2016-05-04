#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
in  vec3 v_norm;
in  vec3 v_lightDir;
out vec3 o_color;
uniform sampler2D s_texture;

void main()
{
  float multiplier = dot(v_lightDir, v_norm);
  multiplier = clamp(multiplier, 0.1f, 1.0f);
	o_color = texture2D(s_texture, vec2(v_texCoord[0], 1.0f - v_texCoord[1])).rgb;
  o_color = o_color * multiplier;
}
