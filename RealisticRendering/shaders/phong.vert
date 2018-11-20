#version 330
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec4 color;
out vec3 lightIntensity;

struct LightInfo {  
  vec4 Position; // Light position in eye coords.
  vec3 La;       // Ambient light intensity
  vec3 Ld;       // Diffuse light intensity
  vec3 Ls;       // Specular light intensity
};
uniform LightInfo light;

struct MaterialInfo {
  vec3 Ka;            // Ambient reflectivity
  vec3 Kd;            // Diffuse reflectivity
  vec3 Ks;            // Specular reflectivity
  float Shininess;    // Specular shininess factor
};
uniform MaterialInfo material;

uniform mat4 modelMat;
uniform mat4 viewMat;
uniform mat3 normalMat;
uniform mat4 projection;

void getEyeSpace (out vec3 norm, out vec4 pos) {
  norm = normalize(normalMat * normal);
  pos = viewMat * modelMat * vec4(position, 1.0);
}

vec3 phongModel(vec4 pos, vec3 norm) {
  vec3 s = normalize(vec3(light.Position - pos));
  vec3 v = normalize(-pos.xyz);
  vec3 r = reflect(-s, norm);
  vec3 ambient = light.La * material.Ka;
  float sDotN = max(dot(s,norm), 0.0);
  vec3 diffuse = light.Ld * material.Kd * sDotN;
  vec3 spec = vec3(0.0);
  if (sDotN > 0.0)
    spec = light.Ls * material.Ks * pow(max(dot(r,v), 0.0), material.Shininess);

  return ambient + diffuse + spec;
}

void main()
{
  vec3 eyeNorm;
  vec4 eyePosition;

  getEyeSpace(eyeNorm, eyePosition);
  lightIntensity = phongModel(eyePosition, eyeNorm);

  gl_Position = projection * viewMat * modelMat * vec4(position, 1.0);
}