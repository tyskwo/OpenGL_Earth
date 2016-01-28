#version 100

// Input vertex data, different for all executions of this shader.
attribute lowp vec3 a_vPos;
attribute lowp vec2 a_tCoord;
uniform mat4 u_mvp;

varying vec2 v_texCoord;

void main()
{ 
  gl_Position = u_mvp * vec4(a_vPos, 1);
  v_texCoord = a_tCoord;
}
