attribute vec4 Position;
attribute vec3 DiffuseMaterial;
attribute vec2 TextureCoord;

uniform mat4 Projection;
uniform mat4 Modelview;
uniform mat4 TextureMatrix;

varying vec4 DestinationColor;
varying vec2 TextureCoordOut;

void main(void)
{
    DestinationColor = vec4(DiffuseMaterial, 1);
    gl_Position = Projection * Modelview * Position;
    vec4 Coord = TextureMatrix * vec4(TextureCoord, 1, 1);
    TextureCoordOut = vec2(Coord);
}