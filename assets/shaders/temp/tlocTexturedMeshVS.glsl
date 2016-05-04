#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 a_vertPos;
in vec2 a_vertTexCoord0;
in mat3 a_vertTBN;

uniform mat4 u_model;
uniform mat3 u_normal;

uniform mat4 u_vp;
uniform mat4 u_view;
uniform vec3 u_lightDir;

out vec2 v_texCoord;
out vec3 v_norm;
out vec3 v_lightDir;

void main()
{ 
  gl_Position = u_vp * u_model * vec4(a_vertPos, 1);

  v_texCoord = a_vertTexCoord0;

  // We need the light to be stationary. We can achieve
  // this by either not transforming the normals or 
  // transforming the light as well as the normals. We
  // will transform both just to be 'correct'
  v_norm    = a_vertTBN[2];
  v_norm    = u_normal * v_norm;

  v_lightDir = normalize(u_lightDir);
  v_lightDir = mat3(u_view) * v_lightDir;
}
