#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
out vec4 o_color;

uniform sampler2D s_texture;
uniform sampler2D s_bright;
uniform sampler2D s_godray;
uniform float     u_exposure;

uniform int		  u_flag;

void main()
{
  vec2 tex_offset = 1.0 / textureSize(s_bright, 0);

  float texCoordX = tex_offset[0];
  float texCoordY = tex_offset[1];

	vec3 hdr    = texture2D(s_texture, vec2(v_texCoord[0], v_texCoord[1])).rgb;
	vec3 bright = texture2D(s_bright, vec2(v_texCoord[0], v_texCoord[1])).rgb;
	vec3 godray = texture2D(s_godray, vec2(v_texCoord[0], v_texCoord[1])).rgb;

  // this is taken from learnopengl.com
  const float gamma = 1.0;
  hdr += bright; // additive blending
  hdr += godray; // additive blending
  // tone mapping
  vec3 result = vec3(1.0) - exp(-hdr * u_exposure);
  // also gamma correct while we're at it       
  result = pow(result, vec3(1.0 / gamma));
  o_color = vec4(result, 1.0);

  if(u_flag == 1) { o_color = vec4(bright, 1.0);; }
  if(u_flag == 2) { o_color = vec4(godray, 1.0);; }
}
