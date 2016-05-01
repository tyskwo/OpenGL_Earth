#version 330 core

	in		vec3	  v_lightDirection;			//the light direction
	in		vec3	  v_vertNormal;				//the vertext normal
	in		vec2	  v_texCoord;				//the texture coordinate for the vertex

	uniform sampler2D moon_diffuse;			//texture that holds the earth map		
	uniform sampler2D moon_normals;			//texture for earth normals

			vec4	  color;					//the color of the sphere
			vec3	  vertNorm_interpolated;	//the interpolated normal from each vertex

	layout (location = 0) out		vec4	   o_color;					//the color to pass to the renderer

	

void main()
{
//get the color for each texture at the given coordinate
	vec4 color_diffuse  = texture2D(moon_diffuse,     vec2(v_texCoord.s,				 1 -v_texCoord.t));
	vec4 color_normals  = texture2D(moon_normals,     vec2(v_texCoord.s,				 1 - v_texCoord.t));
		 color_normals  = (color_normals * 2.0) - 1.0;

//normalize the interpolated normal
	vertNorm_interpolated = normalize(v_vertNormal);

//get the diffuse and specular multipliers
	float diffuseMultiplier  = dot(vertNorm_interpolated * color_normals.rgb, v_lightDirection);
	float specularMultiplier = dot(vertNorm_interpolated,				  v_lightDirection);

//get the interpolated color based on the diffuse texture
	color = color_diffuse * diffuseMultiplier;

//gamma correction
	o_color.r = pow(o_color.r, 1.0 / 2.2);
	o_color.g = pow(o_color.g, 1.0 / 2.2);
	o_color.b = pow(o_color.b, 1.0 / 2.2);
}