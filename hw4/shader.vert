#version 410

layout(location = 0) in vec3 position;

// Data from CPU
uniform mat4 MVP;  // ModelViewProjection Matrix
uniform mat4 MV;   // ModelView idMVPMatrix
uniform vec4 cameraPosition;
uniform float heightFactor;

// Texture-related data
uniform sampler2D rgbTexture;
uniform int widthTexture;
uniform int heightTexture;

// Output to Fragment Shader
out vec2 textureCoordinate;  // For texture-color
out vec3 vertexNormal;       // For Lighting computation
out vec3 ToLightVector;      // Vector from Vertex to Light;
out vec3 ToCameraVector;     // Vector from Vertex to Camera;

void main() {
  // get texture value, compute height
  // compute normal vector using also the heights of neighbor vertices

  // compute toLight vector vertex coordinate in VCS
  // light_position = MV * vec4(widthTexture / 2, widthTexture + heightTexture,
  //                            heightTexture / 2, 1.);
  // ToLightVector = light_position - gl_Position;
  vec3 limunance = vec3(.2126, .7152, .0722);
  textureCoordinate =
      vec2(position.x / widthTexture, 1. - position.z / heightTexture);
  vec4 texture_color = texture(rgbTexture, textureCoordinate);
  gl_Position =
      MVP * vec4(position.x, heightFactor * dot(texture_color.xyz, limunance),
                 position.z, 1.);
}
