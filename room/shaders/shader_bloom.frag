#version 430 core
out vec4 FragColor;

in vec2 texCoord;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float exposition;

void main()
{
  //const float gamma = 2.2;
  vec3 hdrColor = texture(scene, texCoord).rgb;
  vec3 bloomColor = texture(bloomBlur, texCoord).rgb;
  hdrColor += bloomColor; // additive blending
  // tone mapping
  //vec3 result = vec3(1.0) - exp(-hdrColor * exposition);
  // also gamma correct while we're at it
  //result = pow(result, vec3(1.0 / gamma));
  FragColor = vec4(hdrColor, 1.0);
}
