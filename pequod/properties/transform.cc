//
// Created by smadhur on 4/12/2026.
//

#include "transform.h"

#include "glm/common.hpp"
#include "debugger/debugger.h"
#include "glm/detail/func_geometric.inl"

namespace Pequod {
Transform::Transform() : transformations_(0) {
  this->position_ = glm::vec3(0.0f);
  this->interpolated_position_ = glm::vec3(0.0f);
  this->previous_position_ = glm::vec3(0.0f);
  this->velocity_ = glm::vec3(0.0f);
}
Transform::Transform(glm::vec3 position) : transformations_(0) {
  this->position_ = position;
  this->previous_position_ = position;
  this->interpolated_position_ = position;
  this->velocity_ = glm::vec3(0.0f);
}

glm::vec3 Transform::GetPosition() const { return this->position_; }
void Transform::SetPosition(glm::vec3 position) {
  this->position_ = position;
  transformations_.push_back(kTransformPosition);
}

glm::vec3 Transform::GetVelocity() const { return this->velocity_; }
void Transform::SetVelocity(glm::vec3 velocity) {
  this->velocity_ = velocity;
  transformations_.push_back(kTransformLinearVelocity);
}
void Transform::SetRotate(glm::vec3 new_value) { this->rotation_ = new_value; }
void Transform::ChangeRotate(glm::vec3 delta) { rotation_ += delta; }
glm::vec3 Transform::GetRotate() const { return this->rotation_; }
void Transform::SetVelocityMagnitude(float) {
  transformations_.push_back(kTransformLinearMagnitude);
}

void Transform::Move(glm::vec3 movement) { this->position_ += movement; }

glm::vec3 Transform::GetInterpolatedPosition() const {
  return this->interpolated_position_;
}

void Transform::InterpolatePosition(float alpha) {
  float a = glm::clamp(alpha, 0.0f, 1.0f);
  interpolated_position_ = glm::mix(previous_position_, position_, a);
}

void Transform::CaptureTickSnapshot() { previous_position_ = position_; }
std::vector<TransformationType> Transform::GetTransformations() {
  auto transformations = transformations_;
  return transformations;
}
void Transform::ClearTransformations() { transformations_.clear(); }
}  // namespace Pequod