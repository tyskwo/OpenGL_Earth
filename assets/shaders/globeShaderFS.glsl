#version 330 core

	in		vec3	  v_lightDirection;			//the light direction
	in		vec3	  v_vertNormal;				//the vertext normal
	in		vec2	  v_texCoord;				//the texture coordinate for the vertex

	uniform sampler2D earth_diffuse;			//texture that holds the earth map		
	uniform sampler2D earth_specular;			//texture that holds the earth's specular
	uniform sampler2D earth_night;				//texture that holds the earth map at night
	uniform sampler2D earth_clouds;				//texture that holds the earth's clouds
	uniform sampler2D earth_clouds_mask;		//texture that holds the cloud mask
	uniform sampler2D earth_normals;			//texture for earth normals
	uniform float	  u_cloud_shift;			//amount to Shift clouds by

			vec3	  color;					//the color of the sphere
			vec3	  vertNorm_interpolated;	//the interpolated normal from each vertex

			float	  shininess = 50;			//specular lighting values
			float	  specularIntensity = 2;

	out		vec3	   o_color;					//the color to pass to the renderer

	

void main()
{
//get the color for each texture at the given coordinate
	vec3 color_diffuse  = texture2D(earth_diffuse,     vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
	vec3 color_night    = texture2D(earth_night,       vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
	vec3 color_clouds   = texture2D(earth_clouds,      vec2(v_texCoord.s + u_cloud_shift, 1 - v_texCoord.t)).rgb;
	vec3 clouds_mask    = texture2D(earth_clouds_mask, vec2(v_texCoord.s + u_cloud_shift, 1 - v_texCoord.t)).rgb;
	vec3 color_specular = texture2D(earth_specular,    vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
	vec3 color_normals  = texture2D(earth_normals,     vec2(v_texCoord.s, 1 - v_texCoord.t)).rgb;
		 color_normals  = (color_normals * 2.0) - 1.0;

//normalize the interpolated normal
	vertNorm_interpolated = normalize(color_normals * v_vertNormal);

//get the diffuse multiplier
	float diffuseMultiplier = dot(vertNorm_interpolated, v_lightDirection);

//get the interpolated color based on the diffuse texture, night texture, and clouds texture.
	color = (color_diffuse * diffuseMultiplier + color_night * (1.0f - diffuseMultiplier) + color_clouds * (1.0f - clouds_mask) * (diffuseMultiplier));

//get the specular value based on the specular intensity and specular mask.
	vec3 specular = specularIntensity * color_specular * max(pow(max(diffuseMultiplier, 0), shininess), 0) * length(color);
	
//pass the calculated color to the renderer; combine the specular with the interpolated color
	o_color = color + specular;
}