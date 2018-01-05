#version 410

// Output Color
out vec4 color;

// Texture-related data;
uniform sampler2D rgbTexture;

// Data from Vertex Shader
in vec2 textureCoordinate;
in vec3 vertexNormal;    // For Lighting computation
in vec3 ToLightVector;   // Vector from Vertex to Light;
in vec3 ToCameraVector;  // Vector from Vertex to Camera;

void main() {
  // Assignment Constants below
  // get the texture color
  vec4 textureColor = texture(rgbTexture, textureCoordinate, -5);

  // apply Phong shading by using the following parameters
  vec4 ka = vec4(0.25, 0.25, 0.25, 1.0);  // reflectance coeff. for ambient
  vec4 Ia = vec4(0.3, 0.3, 0.3, 1.0);     // light color for ambient
  vec4 Id = vec4(1.0, 1.0, 1.0, 1.0);     // light color for diffuse
  vec4 kd = vec4(1.0, 1.0, 1.0, 1.0);     // reflectance coeff. for diffuse
  vec4 Is = vec4(1.0, 1.0, 1.0, 1.0);     // light color for specular
  vec4 ks = vec4(1.0, 1.0, 1.0, 1.0);     // reflectance coeff. for specular
  int specExp = 100;                      // specular exponent

  // compute ambient component
  vec4 ambient = ka * Ia;

  // compute diffuse component
  float cos_theta = max(dot(ToLightVector, vertexNormal), .0);
  vec4 diffuse = cos_theta * kd * Id;
  // compute specular component
  vec3 h = normalize(ToLightVector + ToCameraVector);
  float cos_alpha = max(dot(vertexNormal, h), .0);
  vec4 specular = ks * pow(cos_alpha, specExp) * Is;

  // compute the color using the following equation
  color = vec4(
      clamp(textureColor.xyz * vec3(ambient + diffuse + specular), 0.0, 1.0),
      1.0);
}
