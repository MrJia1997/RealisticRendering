#version 330

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

in vec3 eyeNorm;
in vec4 eyePosition;
out vec4 fColor;

vec3 phongModel(vec4 pos, vec3 norm) {
  vec3 s = normalize(vec3(light.Position - pos));
  vec3 v = normalize(-pos.xyz);
  vec3 r = reflect(-s, norm);
  vec3 h = normalize(v + s);
  vec3 ambient = light.La * material.Ka;
  float sDotN = max(dot(s,norm), 0.0);
  vec3 diffuse = light.Ld * material.Kd * sDotN;
  vec3 spec = vec3(0.0);
  if (sDotN > 0.0)
    spec = light.Ls * material.Ks * pow(max(dot(h,norm), 0.0), material.Shininess);

  return ambient + diffuse + spec;
}

void main()
{
   if (gl_FrontFacing) {
      fColor = vec4(phongModel(eyePosition, eyeNorm), 1.0);
   }
   else {
      fColor = vec4(phongModel(eyePosition, -eyeNorm), 1.0);
   }
}