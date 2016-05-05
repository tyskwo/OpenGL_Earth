#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
in  vec3 v_norm;
in  vec3 v_lightDir;
in  vec4 v_shadowCoord;
out vec3 o_color;

uniform sampler2D       s_texture;
uniform sampler2DShadow s_shadowMap;
uniform vec2            u_imgDim;

void main()
{
  const float ambient    = 0.1;
  const float epsilon    = 0.00001;
  const vec3  lightColor = vec3(1, 1, 1);

  float diffMult = dot(v_lightDir, v_norm);
  float diffMultClamped = clamp(diffMult, 0.0, 1.0);

	vec3 pixelColor = texture2D(s_texture, vec2(v_texCoord[0], 1.0 - v_texCoord[1])).rgb;
  
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

  o_color = pixelColor * (ambient + (shadowMult * diffMultClamped) );
}
