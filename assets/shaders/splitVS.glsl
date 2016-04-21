#version 330 core

in		vec3 a_vertPos;			//the vertex position
in		vec2 a_vertTexCoord0;	//the vertex texture coordinate

uniform mat4 u_vp;				//the view projection matric=x
uniform mat4 u_model;			//the model matrix

out		vec2 v_texCoord;		//the vertex texture coordinate

void main()
{ 
//get the on screen position and pass the texture coordinate to the fragment shader
	gl_Position = u_vp * u_model * vec4(a_vertPos, 1);
	v_texCoord  = a_vertTexCoord0;
}
