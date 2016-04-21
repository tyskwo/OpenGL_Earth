#version 330 core

	in	vec3 v_vertNormal;			//the vertex normal
	in	vec3 v_lightDirection;		//the light direction
	in	vec2 v_texCoord;			//the vertex position

		vec3 color;					//the color of the sphere
		vec3 vertNorm_interpolated;	//the interpolated normal from each vertex

	out vec3 o_color;				//the color to pass to the renderer

void main()
{
//set the color of the sphere
	color = vec3(3.0, 0.0, 0.0);

//normalize the interpolated normal
	vertNorm_interpolated = normalize(v_vertNormal);

//get the diffuse multiplier
	float diffuseMultiplier = dot(vertNorm_interpolated, v_lightDirection);

//pass the calculated color to the renderer
	o_color = diffuseMultiplier * color;
}
