#version 330 core

// We need one out (used to be g_FragColor)
in  vec2 v_texCoord;
out vec4 o_color;

uniform sampler2D s_texture;

uniform int u_horizontal;

uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
  vec2 tex_offset = 1.0 / textureSize(s_texture, 0); // gets size of single texel

  vec2 TexCoords = v_texCoord;
  vec3 result = texture(s_texture, TexCoords).rgb * weight[0]; // current fragment's contribution

  if(u_horizontal == 1)
  {
    for(int i = 1; i < 5; ++i)
    {
      result += texture(s_texture, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
      result += texture(s_texture, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
    }
  }
  else
  {
    for(int i = 1; i < 5; ++i)
    {
      result += texture(s_texture, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
      result += texture(s_texture, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
    }
  }

  o_color = vec4(result, 1.0);
}
