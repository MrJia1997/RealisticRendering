#version 450
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
out vec4 vColor;
 
uniform mat4 modelToWorld;
uniform mat4 worldToCamera;
uniform mat4 cameraToView;

void main()
{
  gl_Position = cameraToView * worldToCamera * modelToWorld * vec4(position, 1.0);
  vColor = vec4(color);
}