#version 430 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BloomColor;

uniform vec3 color;
uniform float exposition;

void main()
{
  FragColor = vec4(vec3(1.0) - exp(-color * exposition), 1);

  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  if (brightness > 0.2)
    BloomColor = vec4(FragColor.rgb, 1.0);
  else
    BloomColor = vec4(0.0, 0.0, 0.0, 1.0);
}
