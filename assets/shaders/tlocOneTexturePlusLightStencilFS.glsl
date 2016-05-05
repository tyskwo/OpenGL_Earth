#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
uniform sampler2D s_texture;
uniform vec3 u_lightColor = vec3(5, 5, 5);

layout (location = 0) out vec4 o_color;
layout (location = 1) out vec4 o_bright;
layout (location = 2) out vec4 o_stencil;

void main()
{
	o_color = texture2D(s_texture, vec2(v_texCoord[0], 1.0 - v_texCoord[1]));
	o_stencil = o_color * 0.5;
	o_stencil.a = o_color.a;
	o_color = o_color * vec4(u_lightColor, 1.0);
  
	float brightness = dot(o_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0) { o_bright = o_color; }
	else { o_bright = vec4(0, 0, 0, 1); }
}
