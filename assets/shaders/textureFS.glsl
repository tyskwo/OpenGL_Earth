#version 330 core

in  vec2 v_texCoord;

layout (location = 0) out vec4 o_color;
layout (location = 1) out vec4 o_colorBright;

uniform sampler2D texture;


void main()
{
	o_color = texture2D(texture, vec2(v_texCoord[0], v_texCoord[1]));

//Check whether fragment output is higher than threshold, if so output as brightness color
    float brightness = dot(o_color.rgb, vec3(0.2126, 0.7152, 0.0722));

    if(brightness > 1.0) o_colorBright = vec4(o_color.rgb, 1.0);
}