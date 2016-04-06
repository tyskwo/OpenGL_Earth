#version 330 core

	in      vec3 a_vertPos;			//vertex position
	in		vec2 a_vertTexCoord0;	//vertex texture coordinate
	in		mat3 a_vertTBN;			//vertex tbn matrix

	uniform mat4 u_vp;				//the view projection matrix
	uniform mat4 u_model;			//the model

	out		vec2 v_position;		//the vertex texture coordinate
	out		vec2 v_screenPosition;	//the vertex position on the screen

void main()
{ 
//determine the position of the vertex
	gl_Position = u_vp * u_model * vec4(a_vertPos, 1);

//pass the texture coordinate through to the fragment shader
	v_position = a_vertTexCoord0;

//pass the screen coordinates to the fragment shader
	v_screenPosition = gl_Position.xy;
}