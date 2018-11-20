#version 330
in vec3 lightIntensity;
out vec4 fColor;
 
void main()
{
   fColor = vec4(lightIntensity, 1.0);
}