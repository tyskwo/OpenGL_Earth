#version 330 core

	in		vec3 v_vertNormal;			//the vertex normal
	in		vec3 v_lightDirection;		//the light direction
	in		vec2 v_texCoord;			//the vertex position

	uniform vec3 u_lightColor;			//the color of the light
	
	out		vec4 o_color;				//the color to pass to the renderer

void main()
{
//set the color of the sphere
	o_color = vec4(3.0f, 0.0f, 0.0f, 1.0f);

//get the diffuse multiplier
	float diffuseMultiplier = dot(v_vertNormal, v_lightDirection);
		  diffuseMultiplier = clamp(diffuseMultiplier, 0.1f, 1.0f);

//pass the calculated color to the renderer
	o_color = diffuseMultiplier * o_color * vec4(u_lightColor, 1.0f); 
}
