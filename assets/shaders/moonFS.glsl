#version 330 core

	in		vec3	  v_lightDirection;			//the light direction
	in		vec3	  v_vertNormal;				//the vertext normal
	in		vec2	  v_texCoord;				//the texture coordinate for the vertex


	uniform sampler2D moon_diffuse;				//texture that holds the earth map		
	uniform sampler2D moon_normals;				//texture for earth normals
    uniform vec3 u_lightColor = vec3(5, 5, 5);


			vec4	  color;					//the color of the sphere
			vec3	  vertNorm_interpolated;	//the interpolated normal from each vertex
			float	  shininess = 40;			//specular lighting values
			float	  specularIntensity = 1.15f;


	layout (location = 0) out		vec4	   o_color;					//the color to pass to the renderer
	layout (location = 1) out       vec4       o_bright;                //the bright values of the scene

	

void main()
{
//get the color for each texture at the given coordinate
	vec4 color_diffuse  = texture2D(moon_diffuse, vec2(v_texCoord.s, 1 -v_texCoord.t));
	vec4 color_normals  = texture2D(moon_normals, vec2(v_texCoord.s, 1 - v_texCoord.t));
		 color_normals  = (color_normals * 2.0f) - 1.0f;

//normalize the interpolated normal
	vertNorm_interpolated = normalize(v_vertNormal);

//get the diffuse and specular multipliers
	float diffuseMultiplier  = dot(vertNorm_interpolated * color_normals.rgb, v_lightDirection);
	float specularMultiplier = dot(vertNorm_interpolated,				      v_lightDirection);

//get the interpolated color based on the diffuse texture
	color = color_diffuse * diffuseMultiplier;

//get the specular value based on the specular intensity and specular mask.
	vec4 specular = specularIntensity * color_diffuse * max(pow(max(specularMultiplier, 0), shininess), 0) * length(color);
	
//pass the calculated color to the renderer; combine the specular with the interpolated color
	o_color = color + specular;

	o_color = o_color * vec4(u_lightColor, 1.0);
	
	float brightness = dot(o_color.rgb, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > 1.0) { o_bright = o_color; }
	else { o_bright = vec4(0, 0, 0, 1); }

//gamma correction
	/*o_color.r = pow(o_color.r, 1.0f / 2.2f);
	o_color.g = pow(o_color.g, 1.0f / 2.2f);
	o_color.b = pow(o_color.b, 1.0f / 2.2f);*/
	o_color.a = 1.0f;
}