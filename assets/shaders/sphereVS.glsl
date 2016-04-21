#version 330 core

	in      vec3 a_vertPos;			//vertex position
	in      vec3 a_vertNorm;		//vertex normal
	in		vec2 a_vertTexCoord0;	//vertex coordinate
	in		mat3 a_vertTBN;			//the tbn matrix

	uniform mat4 u_vp;				//the view projection matrix
	uniform mat4 u_view;			//???
	uniform mat3 u_normal;			//???
	uniform mat4 u_model;			//the model
	uniform vec3 u_lightDirection;	//the light direction

	out		vec3 v_vertNormal;		//the vertex's normal
	out		vec3 v_lightDirection;	//the light direction
	out		vec2 v_texCoord;		//the vertex position


void main()
{ 
//determine the position of the vertex
	gl_Position = u_vp * u_model * vec4(a_vertPos, 1);

//pass the normal to the fragment shader after transforming world view
	v_vertNormal = a_vertTBN[2] + a_vertNorm;
	v_vertNormal = u_normal * v_vertNormal;

//determine the light direction based on world view
	v_lightDirection = normalize(u_lightDirection);
	v_lightDirection = mat3(u_view) * v_lightDirection;

//pass the vertex through to the fragment shader
	v_texCoord = a_vertTexCoord0;
}
