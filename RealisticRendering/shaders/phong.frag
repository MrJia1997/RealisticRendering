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
in vec4 shadowCoord;
in vec2 texC;

uniform sampler2D texUnit;
uniform sampler2D shadowUnit;

float unpack(vec4 colour) {
  const vec4 bitShifts = vec4(1.0 / (256.0 * 256.0 * 256.0),
                              1.0 / (256.0 * 256.0),
                              1.0 / 256.0,
                              1);
  return dot(colour , bitShifts);
}

float shadowSimple() {
  vec4 shadowMapPosition = shadowCoord / shadowCoord.w;

  shadowMapPosition = (shadowMapPosition + 1.0) /2.0;

  vec4 packedZValue = texture2D(shadowUnit, shadowMapPosition.st);

  float distanceFromLight = unpack(packedZValue);

  //add bias to reduce shadow acne (error margin)
  float bias = 0.0005;

  //1.0 = not in shadow (fragment is closer to light than the value stored in shadow map)
  //0.0 = in shadow
  return float(distanceFromLight > shadowMapPosition.z - bias);
}

vec3 phongModel(vec4 pos, vec3 norm, vec4 shadowC) {
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

  vec3 texColor = vec3(texture2D(texUnit, texC));
  vec3 diffColor = diffuse * texColor;
  vec3 ambColor = ambient * texColor;
  
  float shadow = 1.0;
  if (shadowC.w > 0.0) {
    shadow = shadowSimple();
    shadow = shadow * 0.8 + 0.2;
  }
  
  return shadow * (ambColor + diffColor + spec);
}

void main() {
  gl_FragColor = vec4(phongModel(eyePosition, eyeNorm, shadowCoord), 1.0);
}