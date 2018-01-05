#version 410

layout(location = 0) in vec3 position;

// Data from CPU
uniform mat4 MVP;   // ModelViewProjection Matrix
uniform mat4 MVIT;  // Inverse Transpose of ModelView Matrix
uniform mat4 MV;    // Inverse Transpose of ModelView Matrix
uniform mat4 depthMVP;
uniform vec3 cameraPosition;
uniform vec3 lightPosition;
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
out vec4 depthCoordinate;

const vec3 limunance = vec3(.2126, .7152, .0722);

const int delta[2] = int[2](1, -1);
const int mult[2] = int[2](1, -1);

float GetHeight(vec2 texture_coordinate) {
  vec3 texture_color = texture(rgbTexture, texture_coordinate).xyz;
  return heightFactor * dot(texture_color, limunance);
}

vec3 CalculateSurfaceNormal(vec3 v1, vec3 v2, vec3 v3) {
  vec3 e1 = v3 - v1;
  vec3 e2 = v2 - v1;
  return normalize(cross(e1, e2));
}

bool IsValid(vec3 position) {
  return position.x >= 0 && position.z >= 0 && position.x < widthTexture &&
         position.z < heightTexture;
}

vec2 GetTextureCoordinate(vec3 position) {
  return vec2(1. - position.x / widthTexture, 1. - position.z / heightTexture);
}

void main() {
  vec3 light_position = vec3(MV * vec4(lightPosition, 1.));

  textureCoordinate = GetTextureCoordinate(position);
  vec3 current_vertex = position;
  current_vertex.y = GetHeight(textureCoordinate);

  vec3 current_vertex_vcs = vec3(MV * vec4(current_vertex, 1.));
  ToLightVector = normalize((light_position - current_vertex_vcs).xyz);
  ToCameraVector = normalize((-current_vertex_vcs).xyz);

  vertexNormal = vec3(.0, .0, .0);
  for (int i = 0; i < 2; i++) {
    vec3 neighbour1 = vec3(position.x + delta[i], position.y, position.z);
    if (!IsValid(neighbour1)) continue;
    for (int j = 0; j < 2; j++) {
      vec3 neighbour2 = vec3(position.x, position.y, position.z + delta[j]);
      if (!IsValid(neighbour2)) continue;
      neighbour1.y = GetHeight(GetTextureCoordinate(neighbour1));
      neighbour2.y = GetHeight(GetTextureCoordinate(neighbour2));
      vertexNormal +=
          mult[i] * mult[j] *
          CalculateSurfaceNormal(current_vertex, neighbour1, neighbour2);
    }
  }
  vertexNormal = normalize(vec3(MVIT * vec4(normalize(vertexNormal), .0)));

  gl_Position = MVP * vec4(current_vertex, 1.);
  depthCoordinate = depthMVP * vec4(current_vertex, 1.);
}
