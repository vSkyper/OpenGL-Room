#version 430 core
layout(location = 0) in vec3 vertexPosition;

uniform mat4 viewProjectionMatrix;
uniform mat4 modelMatrix;

void main()
{
  gl_Position = viewProjectionMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}
