#version 330 core

in		vec3 a_vertPos;			//vertex position
in		vec2 a_vertTexCoord0;	//vertex texture coordinate

uniform mat4 u_vp;				//view projection matrix
uniform mat4 u_model;			//model matric

out		vec2 v_texCoord;		//texture coordinate of vertex

void main()
{ 
///get the position on screen and pass through the vertex texture coordinate
	gl_Position = u_vp * u_model * vec4(a_vertPos, 1);
	v_texCoord  = a_vertTexCoord0;
}
