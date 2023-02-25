#version 430 core
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexTexCoord;

uniform mat4 transformation;

out vec2 vecTex;

void main()
{
  gl_Position = transformation * vec4(vertexPosition, 1.0);
  vecTex = vec2(0, 1) + vertexTexCoord * vec2(1, -1);
}
