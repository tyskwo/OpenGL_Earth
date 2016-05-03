#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 a_vertPos;
in vec2 a_vertTexCoord0;

uniform mat4  u_vp;
uniform vec3  u_lightDir;
out vec2      v_texCoord;
out vec3      v_lightPosOnScreen;

void main()
{ 
  gl_Position = vec4(a_vertPos, 1);
  v_texCoord = a_vertTexCoord0;
  vec4 lightPosClip = u_vp * vec4(u_lightDir, 1);
  lightPosClip  = lightPosClip / lightPosClip.w;

  v_lightPosOnScreen = lightPosClip.xyz * 0.5 + 0.5;
}
