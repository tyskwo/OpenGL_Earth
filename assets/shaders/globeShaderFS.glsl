#version 330 core

	in	vec3 v_vertNormal;			//the vertex normal
	in	vec3 v_lightDirection;		//the light direction
	in  vec2 v_texCoord;

		vec3 color;					//the color of the sphere
		vec3 vertNorm_interpolated;	//the interpolated normal from each vertex

	out vec3 o_color;				//the color to pass to the renderer

	uniform sampler2D s_texture;
	uniform sampler2D s_texture_2;

void main()
{
	vec3 color_1 = texture2D(s_texture, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;

	vec3 color_2 = texture2D(s_texture_2, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;

	vec3 col1Inv = vec3(1,1,1) - color_1;

	color = color_2;
	color = (color * col1Inv) + color_1;





//normalize the interpolated normal
	vertNorm_interpolated = normalize(v_vertNormal);

//get the diffuse multiplier
	float diffuseMultiplier = dot(vertNorm_interpolated, v_lightDirection);

//pass the calculated color to the renderer
	o_color = diffuseMultiplier * color;
}