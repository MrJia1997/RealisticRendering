#version 440
layout(location = 0) in vec3 position;

uniform mat4 modelMat;
uniform mat4 lightViewProjMat;

out vec4 projPos;

void main() {
  projPos = lightViewProjMat * modelMat * vec4(position, 1.0);
  gl_Position = projPos;
}