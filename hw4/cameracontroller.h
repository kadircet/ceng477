#ifndef CAMERACONTROLLER_H_
#define CAMERACONTROLLER_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

class CameraController {
 public:
  CameraController() {}
  CameraController(const glm::vec3& position, const glm::vec3& gaze,
                   const glm::vec3& up, const float fovy, const float aspect,
                   const float near, const float far)
      : proj_matrix_(glm::perspective(fovy, aspect, near, far)),
        position_(position),
        gaze_(gaze),
        up_(up),
        height_factor_(10.f) {
    Move(0);
    UpdateMatrices();
  }

  const glm::mat4& GetView() const { return view_matrix_; }
  const glm::mat4& GetProj() const { return proj_matrix_; }
  const glm::mat4& GetMVP() const { return mvp_matrix_; }
  const glm::mat4& GetMVIT() const { return mvit_matrix_; }
  const glm::vec3& GetPosition() const { return position_; }
  const float& GetHeightFactor() const { return height_factor_; }

  void Move(const float unit);
  void IncrementHeight() { height_factor_ += .5; }
  void DecrementHeight() { height_factor_ -= .5; }
  void SetPositionY(const float new_y) {
    position_.y = new_y;
    UpdateMatrices();
  }

 private:
  void UpdateMatrices();

  glm::mat4 view_matrix_;
  glm::mat4 proj_matrix_;
  glm::mat4 mvp_matrix_;
  glm::mat4 mvit_matrix_;

  glm::vec3 position_;
  glm::vec3 gaze_;
  glm::vec3 up_;

  float height_factor_;
};

void CameraController::Move(const float unit) {
  position_ += unit * gaze_;
  UpdateMatrices();
}

void CameraController::UpdateMatrices() {
  view_matrix_ = glm::lookAt(position_, position_ + gaze_, up_);
  mvp_matrix_ = proj_matrix_ * view_matrix_;
  mvit_matrix_ = inverseTranspose(view_matrix_);
}

#endif
