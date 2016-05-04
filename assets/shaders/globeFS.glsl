#version 330 core

	in		vec3	  v_lightDirection;			//the light direction
	in		vec3	  v_vertNormal;				//the vertext normal
	in		vec2	  v_texCoord;				//the texture coordinate for the vertex


	uniform sampler2D earth_diffuse;			//texture that holds the earth map		
	uniform sampler2D earth_specular;			//texture that holds the earth's specular
	uniform sampler2D earth_night;				//texture that holds the earth map at night
	uniform sampler2D earth_clouds;				//texture that holds the earth's clouds
	uniform sampler2D earth_normals;			//texture for earth normals
	uniform sampler2D water_normals;			//texture for earth normals

	uniform float	  u_cloudAngle;				//amount to shift clouds by

	uniform int		  cloud_flag;			    //flag for whether to draw the clouds


			vec4	  color;					//the color of the sphere
			vec3	  vertNorm_interpolated;	//the interpolated normal from each vertex

			float	  shininess = 60;			//specular lighting values
			float	  specularIntensity = 1.1;

	out		vec4	   o_color;					//the color to pass to the renderer

	
//random value function
float random(vec2 value)
{
	return fract(sin(dot(value.xy, vec2(2.898, 8.233))) * 358.5453) * 10;
}

void main()
{
//get the color for each texture at the given coordinate
	vec4 color_diffuse			= texture2D(earth_diffuse,		vec2(v_texCoord.s,						1 - v_texCoord.t));
	vec4 color_night			= texture2D(earth_night,		vec2(v_texCoord.s,						1 - v_texCoord.t));
	vec4 color_clouds			= texture2D(earth_clouds,		vec2(v_texCoord.s + u_cloudAngle,		1 - v_texCoord.t));
	vec4 color_specular			= texture2D(earth_specular,		vec2(v_texCoord.s,						1 - v_texCoord.t));
	vec4 color_normals			= texture2D(earth_normals,		vec2(v_texCoord.s,						1 - v_texCoord.t));
	vec4 water_color_normals1	= texture2D(water_normals,		vec2(v_texCoord.s + 2 * u_cloudAngle,	1 - v_texCoord.t));
	vec4 water_color_normals2	= texture2D(water_normals,		vec2(v_texCoord.s - 2 * u_cloudAngle,	1 - v_texCoord.t));
	vec4 water_color_normals3	= texture2D(water_normals,		vec2(v_texCoord.s,						1 - v_texCoord.t - 2*u_cloudAngle));
	vec4 water_color_normals4	= texture2D(water_normals,		vec2(v_texCoord.s,						1 - v_texCoord.t + 2*u_cloudAngle));
	vec4 water_color_normals5	= texture2D(water_normals,		vec2(v_texCoord.s - 2*u_cloudAngle,		1 - v_texCoord.t - 2*u_cloudAngle));
	vec4 water_color_normals6	= texture2D(water_normals,		vec2(v_texCoord.s + 2*u_cloudAngle,		1 - v_texCoord.t + 2*u_cloudAngle));

//correct normals
	color_normals  = (color_normals * 2.0f) - 1.0f;

	water_color_normals1  = (water_color_normals1 * 2.0f) - 1.0f;
	water_color_normals2  = (water_color_normals2 * 2.0f) - 1.0f;
	water_color_normals3  = (water_color_normals3 * 2.0f) - 1.0f;
	water_color_normals4  = (water_color_normals4 * 2.0f) - 1.0f;
	water_color_normals5  = (water_color_normals5 * 2.0f) - 1.0f;
	water_color_normals6  = (water_color_normals6 * 2.0f) - 1.0f;
	
//calculate water normal average
	vec4 water_color_normals = water_color_normals1 + water_color_normals2 + water_color_normals3 + water_color_normals4 + water_color_normals5 + water_color_normals6;
		 water_color_normals = water_color_normals / 6.0f;

//normalize the interpolated normal
	vertNorm_interpolated = normalize(v_vertNormal);

//get the diffuse and specular multipliers
	float diffuseMultiplier  = dot(vertNorm_interpolated * color_normals.rgb, v_lightDirection);
	float waterDiffuseMultiplier  = dot(vertNorm_interpolated * water_color_normals.rgb, v_lightDirection);
	float specularMultiplier = dot(vertNorm_interpolated,				  v_lightDirection);

	vec4 land = color_diffuse * diffuseMultiplier * abs(color_specular - 1.0f);
	vec4 sea = color_diffuse * color_specular * waterDiffuseMultiplier;
	vec4 night = color_night * (1.0f - diffuseMultiplier);
	vec4 clouds = color_clouds * (diffuseMultiplier);

//get the interpolated color based on the diffuse texture, night texture, and clouds texture (if clouds are to be drawn)
	if(cloud_flag == 0)
	{ color = land + sea + night + clouds; }
	else
	{ color = land + sea + night; }

//get the specular value based on the specular intensity and specular mask.
	vec4 specular = specularIntensity * color_specular * max(pow(max(specularMultiplier, 0), shininess), 0) * length(color);
	
//pass the calculated color to the renderer; combine the specular with the interpolated color
	o_color = color + specular;

//gamma correction
	o_color.r = pow(o_color.r, 1.0f / 2.2f);
	o_color.g = pow(o_color.g, 1.0f / 2.2f);
	o_color.b = pow(o_color.b, 1.0f / 2.2f);
	o_color.a = 1.0f;
}