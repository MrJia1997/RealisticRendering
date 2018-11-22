#version 440
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 texCoord;

out vec3 eyeNorm;
out vec4 eyePosition;
out vec4 shadowCoord;
out vec2 texC;

uniform sampler2D dispUnit;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat3 normalMat;
uniform mat4 projection;
uniform mat4 lightViewProjMat;

void getEyeSpace (out vec3 norm, out vec4 pos) {
  norm = normalize(normalMat * normal);
  pos = viewMat * modelMat * vec4(position, 1.0);
}

void main()
{
  getEyeSpace(eyeNorm, eyePosition);
  texC = vec2(texCoord);
  shadowCoord = lightViewProjMat * modelMat * vec4(position, 1.0);
  vec4 disp = texture2D(dispUnit, texC);
  disp = normalize(disp * 2.0 - 1.0);
  vec4 pos = vec4(position, 1.0) + 0.05 * disp;
  gl_Position = projection * viewMat * modelMat * pos;
}