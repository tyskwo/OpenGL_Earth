#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
in  vec3 v_lightPosOnScreen;

out vec4 o_color;

uniform sampler2D s_stencil;

uniform int   u_numSamples  = 200;
uniform float u_density     = 0.5;
uniform float u_decay       = 0.9;
uniform float u_weight      = 0.5;
uniform float u_exposure    = 0.1;
uniform float u_illumDecay  = 1.0;

void main()
{
  vec2 lightPosScreen = v_lightPosOnScreen.xy;

  vec2 deltaTexCoord = vec2(v_texCoord - lightPosScreen);
  deltaTexCoord = normalize(deltaTexCoord);
  vec2 texCoo = v_texCoord;
  deltaTexCoord *= 1.0 / float(u_numSamples) * u_density;

  float illumDec = u_illumDecay;

	vec4 rays = vec4(0, 0, 0, 1);

  for (int i = 0; i < u_numSamples; i++)
  {
    texCoo -= deltaTexCoord;
    vec4 sample = texture2D(s_stencil, texCoo);
    sample *= illumDec * u_weight;
    rays += sample;
    illumDec *= u_decay;
  }

  o_color = rays * u_exposure;
}
