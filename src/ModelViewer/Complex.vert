attribute vec4 Position;
attribute vec3 Normal;
attribute vec3 DiffuseMaterial;
attribute vec2 TextureCoord;

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat3 NormalMatrix;
uniform mat4 TextureMatrix;
uniform vec3 LightPosition;
uniform vec3 AmbientMaterial;
uniform vec3 SpecularMaterial;
uniform float Shininess;

varying vec4 DestinationColor;
varying vec2 TextureCoordOut;


void main(void)
{
   vec4 VP = Modelview * Position;
   vec3 N = normalize(NormalMatrix * Normal);
   vec3 LV = LightPosition - vec3(VP);
   vec3 L = normalize(LV);
   vec3 E = vec3(0, 0, 1);
   vec3 H = normalize(L + E);
   
   float distFactor = dot(LV, LV);
   distFactor = distFactor * distFactor;
   //distFactor = inversesqrt(distFactor);
   
   float df = max(0.0, dot(N, L));
   df = df / ( 1.0 + 0.000005 * distFactor);
   
   float sf = max(0.0, dot(N, H));
   sf = pow(sf, Shininess);
   
   sf = sf / ( 1.0 + 0.000005 * distFactor);
   
   vec3 color = AmbientMaterial + df * DiffuseMaterial + sf * SpecularMaterial;
   
   DestinationColor = vec4(color, 1);
   gl_Position = Projection * VP;
   vec4 Coord = TextureMatrix * vec4(TextureCoord, 1, 1);
   TextureCoordOut = vec2(Coord);
}