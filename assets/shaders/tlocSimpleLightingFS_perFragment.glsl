#version 330 core

// We need one out (used to be g_FragColor)
in vec3 v_vertPos;
in vec3 v_vertNorm;

out vec3 o_color;

uniform vec3 u_lightPosition;

void main()
{
	vec3 vertNorm = normalize(v_vertNorm);

	vec3 col = vec3(1.0, 0.0, 0.0);

	vec3 lightDir = u_lightPosition - v_vertPos;
	lightDir = normalize(lightDir);

	float diffMult = dot(vertNorm, lightDir);
	col = diffMult * col;

	o_color = col;
}
