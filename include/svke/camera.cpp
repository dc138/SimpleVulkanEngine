#include "camera.hpp"

namespace svke {
  void Camera::UseOrthographicProjection(float left, float right, float top, float bottom, float near, float far) {
    pProjectionMatrix       = glm::mat4 {1.0f};
    pProjectionMatrix[0][0] = 2.f / (right - left);
    pProjectionMatrix[1][1] = 2.f / (bottom - top);
    pProjectionMatrix[2][2] = 1.f / (far - near);
    pProjectionMatrix[3][0] = -(right + left) / (right - left);
    pProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    pProjectionMatrix[3][2] = -near / (far - near);
  }

  void Camera::UsePerspectiveProjection(float fov_y, float aspect_ratio, float near, float far) {
    assert(glm::abs(aspect_ratio - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tan_half_fov_y = tan(fov_y / 2.f);

    pProjectionMatrix       = glm::mat4 {0.0f};
    pProjectionMatrix[0][0] = 1.f / (aspect_ratio * tan_half_fov_y);
    pProjectionMatrix[1][1] = 1.f / (tan_half_fov_y);
    pProjectionMatrix[2][2] = far / (far - near);
    pProjectionMatrix[2][3] = 1.f;
    pProjectionMatrix[3][2] = -(far * near) / (far - near);
  }
}
