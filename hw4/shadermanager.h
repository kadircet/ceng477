#ifndef SHADERMANAGER_H_
#define SHADERMANAGER_H_

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "helper.h"

#include <unordered_map>

class ShaderManager {
 public:
  ShaderManager() {}
  ShaderManager(const string& vertex_shader, const string& fragment_shader);
  void Render(const GLenum mode, const GLsizei count, const GLenum type,
              const void* indices);

  template <typename T>
  void Update(const string& key, const T& val);
  const GLuint& GetProgramID() const { return program_id_; }
  void UseShader() const { glUseProgram(program_id_); }

 private:
  GLuint GetOrCreateId(const string& key);
  std::unordered_map<string, GLuint> uniform_variables_;
  GLuint program_id_;
};

ShaderManager::ShaderManager(const string& vertex_shader,
                             const string& fragment_shader) {
  program_id_ = initShaders(vertex_shader, fragment_shader);
}

void ShaderManager::Render(const GLenum mode, const GLsizei count,
                           const GLenum type, const void* indices) {
  UseShader();
  glDrawElements(mode, count, type, indices);
}

GLuint ShaderManager::GetOrCreateId(const string& key) {
  auto it = uniform_variables_.find(key);
  if (it == uniform_variables_.end()) {
    it = uniform_variables_
             .insert({key, glGetUniformLocation(program_id_, key.c_str())})
             .first;
  }
  return it->second;
}

template <>
void ShaderManager::Update<glm::vec3>(const string& key, const glm::vec3& val) {
  const GLuint id = GetOrCreateId(key);
  glUniform3f(id, val.x, val.y, val.z);
}

template <>
void ShaderManager::Update<glm::mat4>(const string& key, const glm::mat4& val) {
  const GLuint id = GetOrCreateId(key);
  glUniformMatrix4fv(id, 1, GL_FALSE, &val[0][0]);
}

template <>
void ShaderManager::Update<int>(const string& key, const int& val) {
  const GLuint id = GetOrCreateId(key);
  glUniform1i(id, val);
}

template <>
void ShaderManager::Update<float>(const string& key, const float& val) {
  const GLuint id = GetOrCreateId(key);
  glUniform1f(id, val);
}

#endif
