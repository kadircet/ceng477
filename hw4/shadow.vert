#version 410

layout(location = 0) in vec3 position;

uniform mat4 MVP;
uniform float heightFactor;

uniform sampler2D rgbTexture;
uniform int widthTexture;
uniform int heightTexture;

const vec3 limunance = vec3(.2126, .7152, .0722);

vec2 GetTextureCoordinate(vec3 position) {
  return vec2(1. - position.x / widthTexture, 1. - position.z / heightTexture);
}

float GetHeight(vec2 texture_coordinate) {
  vec3 texture_color = texture(rgbTexture, texture_coordinate).xyz;
  return heightFactor * dot(texture_color, limunance);
}

void main() {
  vec3 current_vertex = position;
  current_vertex.y = GetHeight(GetTextureCoordinate(position));
  gl_Position = MVP * vec4(current_vertex, 1);
}
