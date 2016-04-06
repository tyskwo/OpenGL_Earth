#version 330 core

	in		vec2	  v_position;		//the texture coordinate for the vertex
	in		vec2	  v_screenPosition;	//the screen coordinate of the vertex

	uniform sampler2D skybox_diffuse;	//texture that holds the skybox	
	uniform float     u_twinkleTime;	//current time to alter the twinkle

			vec3	  color;			//intermediary color

	out		vec3	  o_color;			//the color to pass to the renderer

	

void main()
{
//get the color from the texture at the given coordinate
	color = texture2D(skybox_diffuse, vec2(v_position.s, 1 - v_position.t)).rgb;

//twinkle the stars based on their position
	color *= abs(cos(u_twinkleTime + v_screenPosition.x));

//pass the color to the renderer
	o_color = color;
}