#version 430 core
out vec4 FragColor;

in vec2 tc;

uniform sampler2D depthMap;

float rescale_z(float z) {
  float n = 0.05;
  float f = 20.;
  return (2 * n * f / (z * (n - f) + n + f)) / f;
}

void main()
{
  float depthValue = texture(depthMap, tc).r;
  FragColor = vec4(vec3(rescale_z(depthValue) + 0.5), 1.0);
}
