#version 330 core

	in      vec3 a_vertPos;			//vertex position
	in      vec3 a_vertNorm;		//vertex normal

	uniform mat4 u_vp;				//the view projection matrix
	uniform mat4 u_model;			//the model
	uniform vec3 u_lightPosition;	//the light position

	out		vec3 v_vertNormal;		//the vertex's normal
	out		vec3 v_lightDirection;	//the light direction

void main()
{ 
//determine the position of the vertex
	gl_Position = u_vp * u_model * vec4(a_vertPos, 1);

//transform into world view
	vec3 vertexWorldPosition = (u_model * vec4(a_vertPos, 1)).xyz;

//determine the light direction based on world view
	vec3 lightDirection = u_lightPosition - vertexWorldPosition;
	v_lightDirection	= normalize(lightDirection);

//pass the normal to the fragment shader after transforming world view
	mat3 rotationMatrix = mat3(u_model);

	v_vertNormal = rotationMatrix * a_vertNorm;
}