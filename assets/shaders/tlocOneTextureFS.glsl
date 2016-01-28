#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
out vec3 o_color;
uniform sampler2D s_texture;

void main()
{
  // NOTE: Tex co-ords flipped in 't'
	o_color = texture2D(s_texture, 
                      vec2(v_texCoord.s,
                           1 - v_texCoord.t)
                     ).rgb;
}
