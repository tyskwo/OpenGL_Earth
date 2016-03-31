#version 330 core

	in		vec2	  v_texCoord;		//the texture coordinate for the vertex

	uniform sampler2D skybox_diffuse;	//texture that holds the skybox	

	out		vec3	  o_color;			//the color to pass to the renderer

	

void main()
{
//get the color from the texture at the given coordinate
	o_color  = texture2D(skybox_diffuse, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
}