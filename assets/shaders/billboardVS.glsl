#version 330 core

in		vec3 a_vertPos;			//the vertex position
in		vec2 a_vertTexCoord0;	//the texture coordinate for the vertex

uniform mat4 u_proj;			//the projection matrix
uniform mat4 u_view;			//the view matrix
uniform mat4 u_model;			//the model matrix

out		vec2 v_texCoord;		//the texture coordinate to pass to the fragment shader

void main()
{ 
//determine view-model matrix and set to rotate towards the camera vector
	mat4 vm = u_view * u_model;
		 vm[0].xyz = vec3(1, 0, 0);
		 vm[1].xyz = vec3(0, 1, 0);
		 vm[2].xyz = vec3(0, 0, 1);
  
	gl_Position = u_proj * vm * vec4(a_vertPos, 1);

//pass the texture coordinate through
	v_texCoord = a_vertTexCoord0;
}
