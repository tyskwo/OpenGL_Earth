#version 100

// We need one out (used to be g_FragColor)
varying lowp vec2 v_texCoord;

uniform sampler2D s_texture;

void main()
{
	gl_FragColor.rgb = texture2D(s_texture, vec2(v_texCoord[0], 1.0 - v_texCoord[1])).rgb;
}
