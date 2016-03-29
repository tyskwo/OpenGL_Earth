#version 330 core

	in		vec3 v_vertNormal;			//the vertex normal
	in		vec3 v_lightDirection;		//the light direction
	in		vec2 v_texCoord;			//the texture coordinate for the vertex

	uniform sampler2D earth_diffuse;	//texture that holds the earth map		
	uniform sampler2D earth_specular;	//texture that holds the earth's specular
	uniform sampler2D earth_night;		//texture that holds the earth map at night

			vec3 color;					//the color of the sphere
			vec3 vertNorm_interpolated;	//the interpolated normal from each vertex

	out		vec3 o_color;				//the color to pass to the renderer

	

void main()
{
//get the color based on the two passed in textures.
	vec3 color_diffuse  = texture2D(earth_diffuse,  vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
	vec3 color_night    = texture2D(earth_night,    vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
	vec3 color_specular = texture2D(earth_specular, vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;

	vec3 diffuse_inverse = vec3(1,1,1) - color_diffuse;

	color = color_specular;
	color = (color * diffuse_inverse) + color_diffuse;


	//color = (color_specular - color_diffuse) * when_eq(color_specular.r, 1.0);
	//color = (color_diffuse) * when_eq(color_specular.r, 0.0);

	//(Earth * illumination + night * (1-illumination)) * (1-cloud_alpha) + cloud_color * illumination * cloud_alpha

//normalize the interpolated normal
	vertNorm_interpolated = normalize(v_vertNormal);

//get the diffuse multiplier
	float diffuseMultiplier = dot(vertNorm_interpolated, v_lightDirection);

//pass the calculated color to the renderer
	o_color = (color_diffuse * diffuseMultiplier + color_night * (1.0f - diffuseMultiplier));
}