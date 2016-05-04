#version 330 core

	in      vec3 a_vertPos;			//vertex position
	in		vec2 a_vertTexCoord0;	//vertex texture coordinate
	in		mat3 a_vertTBN;			//vertex tbn matrix

	uniform mat4 u_view;			//the view matrix
	uniform mat4 u_proj;			//the projection matrix

	out		vec2 v_position;		//the vertex texture coordinate
	out		vec2 v_screenPosition;	//the vertex position on the screen
	out		mat3 v_vertTBN;			//the vertex tbn matrix

void main()
{ 
//determine the position of the vertex
	gl_Position = u_proj * vec4(mat3(u_view) * vec3(a_vertPos), 1);

//pass the texture coordinate through to the fragment shader
	v_position = a_vertTexCoord0;

//pass the screen coordinates to the fragment shader
	v_screenPosition = gl_Position.xy;

//pass the tbn matrix to the fragment shader
	v_vertTBN = a_vertTBN;
}