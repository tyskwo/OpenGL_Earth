#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 a_vertPos;
in vec2 a_vertTexCoord0;

uniform mat4 u_proj;
uniform mat4 u_view;
uniform mat4 u_model;

out vec2 v_texCoord;

void main()
{ 
  mat4 vm = u_view * u_model;
  vm[0].xyz = vec3(1, 0, 0);
  vm[1].xyz = vec3(0, 1, 0);
  vm[2].xyz = vec3(0, 0, 1);
  
  gl_Position = u_proj * vm * vec4(a_vertPos, 1);

  v_texCoord = a_vertTexCoord0;
}
