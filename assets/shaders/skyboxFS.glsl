#version 330 core

	in		vec2	  v_position;		//the texture coordinate for the vertex
	in		vec2	  v_screenPosition;	//the screen coordinate of the vertex
	in		mat3	  v_vertTBN;		//the tbn matric of the vertex

	uniform sampler2D skybox_diffuse;	//texture that holds the skybox	
	uniform float     u_twinkleTime;	//current time to alter the twinkle
	uniform vec3	  stencil_color;	//stencil color

			vec3	  color;			//intermediary color

	layout (location = 0) out		vec3	  o_color;			//the color to pass to the renderer
	layout (location = 2) out		vec4      o_stencil;                //the bright values of the scene



//random hash table. http://stackoverflow.com/questions/12964279/whats-the-origin-of-this-glsl-rand-one-liner
float random(vec2 value)
{
	return fract(sin(dot(value.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
//get the color from the texture at the given coordinate
	color = texture2D(skybox_diffuse, vec2(v_position.s, 1 - v_position.t)).rgb;

//twinkle the stars based on their position														
	color *= abs(cos(u_twinkleTime + random(v_screenPosition.xx) + random(v_screenPosition.yy) + random(v_vertTBN[2].zx)));

//pass the color to the renderer
	o_color = color;
	o_stencil = vec4(stencil_color,1);
}