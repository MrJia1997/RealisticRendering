#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
out vec3 eyeNorm;
out vec4 eyePosition;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat3 normalMat;
uniform mat4 projection;

void getEyeSpace (out vec3 norm, out vec4 pos) {
  norm = normalize(normalMat * normal);
  pos = viewMat * modelMat * vec4(position, 1.0);
}

void main()
{
  getEyeSpace(eyeNorm, eyePosition);
  
  gl_Position = projection * viewMat * modelMat * vec4(position, 1.0);
}