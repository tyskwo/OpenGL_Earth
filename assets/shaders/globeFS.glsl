#version 330 core

	in		vec3	  v_lightDirection;			//the light direction
	in		vec3	  v_vertNormal;				//the vertext normal
	in		vec2	  v_texCoord;				//the texture coordinate for the vertex
in  vec4 v_shadowCoord;


	uniform sampler2D earth_diffuse;			//texture that holds the earth map		
	uniform sampler2D earth_specular;			//texture that holds the earth's specular
	uniform sampler2D earth_night;				//texture that holds the earth map at night
	uniform sampler2D earth_clouds;				//texture that holds the earth's clouds
	uniform sampler2D earth_normals;			//texture for earth normals
	uniform sampler2D water_normals;			//texture for earth normals

    uniform vec3	  u_lightColor;				//bloom values
	uniform float	  shininess;
	uniform float	  specularIntensity;

	uniform float	  u_cloudAngle;				//amount to shift clouds by

	uniform int		  cloud_flag;			    //flag for whether to draw the clouds
uniform sampler2DShadow s_shadowMap;
uniform vec2            u_imgDim;

			vec4	  color;					//the color of the sphere
			vec3	  vertNorm_interpolated;	//the interpolated normal from each vertex


	layout (location = 0) out		vec4	   o_color;					//the color to pass to the renderer
	layout (location = 1) out       vec4       o_bright;                //the bright values of the scene

	
//random value function
float random(vec2 value)
{
	return fract(sin(dot(value.xy, vec2(2.898, 8.233))) * 358.5453) * 10;
}

void main()
{
	//---------------------------------------------------------------------------
	const float ambient    = 0.1;
  const float epsilon    = 0.00001;
	//---------------------------------------------------------------------------





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
	float diffuseMultiplier       = clamp(dot(vertNorm_interpolated * color_normals.rgb,			v_lightDirection), 0.0, 1.0);
	float waterDiffuseMultiplier  = clamp(dot(vertNorm_interpolated * water_color_normals.rgb,	v_lightDirection), 0.0, 1.0);
	float specularMultiplier      = dot(vertNorm_interpolated,								v_lightDirection);





	//---------------------------------------------------------------------------


	vec3 positionLtNDC  = v_shadowCoord.xyz / v_shadowCoord.w;

  vec2 UVCoords;
  UVCoords.x = positionLtNDC.x;
  UVCoords.y = positionLtNDC.y;
  float z    = positionLtNDC.z + epsilon;

  float xOffset = 1.0/u_imgDim.x;
  float yOffset = 1.0/u_imgDim.y;

  float shadowMult = 0.0;
  int   pcfR       = 6;
  
  for (int y = -pcfR; y <= pcfR; y++)
  {
    for (int x = -pcfR; x <= pcfR; x++)
    {
      vec2 offsets = vec2(x * xOffset, y * yOffset);
      vec3 UVC = vec3(UVCoords + offsets, z);
      shadowMult += texture(s_shadowMap, UVC);
    }
  }

  float pcfRSq = float(pcfR + pcfR) + 1.0;
  pcfRSq = pcfRSq * pcfRSq;
  shadowMult = (shadowMult / pcfRSq);
	//---------------------------------------------------------------------------



	//(ambient + (shadowMult * diffuseMultiplier))
	//(ambient + (shadowMult * waterDiffuseMultiplier))

//get the correct colors from each of the textures and the appropriate multipliers
	vec4 land	= color_diffuse * (ambient + (shadowMult * diffuseMultiplier)) * abs(color_specular - 1.0f);
	vec4 sea	= color_diffuse * color_specular * (ambient + (shadowMult * waterDiffuseMultiplier));
	vec4 night	= color_night * (1.0f - diffuseMultiplier);
	vec4 clouds = color_clouds * ((ambient + (shadowMult * diffuseMultiplier)));

//get the interpolated color based on the diffuse texture, night texture, and clouds texture (if clouds are to be drawn)
	if(cloud_flag == 0)
	{ color = land + sea + night + clouds; }
	else
	{ color = land + sea + night; }

//get the specular value based on the specular intensity and specular mask.
	vec4 specular = specularIntensity * color_specular * max(pow(max(specularMultiplier, 0), shininess), 0) * length(color);
	
//pass the calculated color to the renderer; combine the specular with the interpolated color
	o_color = color + specular;
	o_color = o_color * vec4(u_lightColor, 1.0);
	o_color.a = 1.0f;


	float brightness = dot(o_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0)	{ o_bright = o_color; }
	else					{ o_bright = vec4(0, 0, 0, 1); }
}