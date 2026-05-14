//
// Created by smadhur on 5/8/2026.
//

#ifndef PEQUOD_ENGINE_CAMERA_H
#define PEQUOD_ENGINE_CAMERA_H
#include "property.h"
#include <glm/glm.hpp>

namespace Pequod {
class Camera : public Property {
 public:
  Camera();
  ~Camera();
  glm::mat4 GetView(glm::vec3);
  glm::mat4 GetProjection(float width, float height);
  void SetZoom(float zoom);
  float GetZoom();
  glm::vec3 GetDirection();
  glm::vec3 GetInterpolatedDirection() const;
  void SetDirection(glm::vec3 direction);

  void SetFOV(float);

  float GetYaw();
  float GetPitch();
  void SetYaw(float);
  void UpdateYaw(float);
  void SetPitch(float);
  void UpdatePitch(float);

  // Same fixed-timestep interpolation pattern as Transform: snapshot at the
  // start of each tick, then lerp every frame so the view doesn't snap when
  // gameplay code rotates yaw/pitch in OnTick.
  void CaptureTickSnapshot();
  void Interpolate(float alpha);

  glm::vec3 GetRayFromScreen(glm::vec2 cursor, glm::vec2 res, glm::vec3 pos);

 private:
  glm::vec3 direction_;
  glm::mat4 view_{};
  glm::mat4 proj_{};
  float yaw = 45.0f;
  float pitch = -30.0f;
  float previous_yaw_ = 45.0f;
  float previous_pitch_ = -30.0f;
  float interpolated_yaw_ = 45.0f;
  float interpolated_pitch_ = -30.0f;

  float fov_ = 60.0f;

  float coord[3] = {0.0, 100.0, 0.0};
  float zoom_ = 1.0f;
};
}  // namespace Pequod

#endif  // PEQUOD_ENGINE_CAMERA_H
