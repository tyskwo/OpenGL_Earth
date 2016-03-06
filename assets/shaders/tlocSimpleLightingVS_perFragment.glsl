#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 a_vertPos;
in vec3 a_vertNorm;

uniform mat4 u_vp;
uniform mat4 u_model;

out vec3 v_vertPos;
out vec3 v_vertNorm;

void main()
{ 
	gl_Position = u_vp * u_model * vec4(a_vertPos, 1);

	v_vertPos = a_vertPos;
	v_vertNorm = a_vertNorm;
}
