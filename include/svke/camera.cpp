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

  void Camera::SetViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up_direction) {
    const glm::vec3 w {glm::normalize(direction)};
    const glm::vec3 u {glm::normalize(glm::cross(w, up_direction))};
    const glm::vec3 v {glm::cross(w, u)};

    pViewMatrix       = glm::mat4 {1.f};
    pViewMatrix[0][0] = u.x;
    pViewMatrix[1][0] = u.y;
    pViewMatrix[2][0] = u.z;
    pViewMatrix[0][1] = v.x;
    pViewMatrix[1][1] = v.y;
    pViewMatrix[2][1] = v.z;
    pViewMatrix[0][2] = w.x;
    pViewMatrix[1][2] = w.y;
    pViewMatrix[2][2] = w.z;
    pViewMatrix[3][0] = -glm::dot(u, position);
    pViewMatrix[3][1] = -glm::dot(v, position);
    pViewMatrix[3][2] = -glm::dot(w, position);
  }

  void Camera::SetViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up_direction) {
    SetViewDirection(position, target - position, up_direction);
  }

  void Camera::SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation) {
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);

    const glm::vec3 u {(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::vec3 v {(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::vec3 w {(c2 * s1), (-s2), (c1 * c2)};

    pViewMatrix       = glm::mat4 {1.f};
    pViewMatrix[0][0] = u.x;
    pViewMatrix[1][0] = u.y;
    pViewMatrix[2][0] = u.z;
    pViewMatrix[0][1] = v.x;
    pViewMatrix[1][1] = v.y;
    pViewMatrix[2][1] = v.z;
    pViewMatrix[0][2] = w.x;
    pViewMatrix[1][2] = w.y;
    pViewMatrix[2][2] = w.z;
    pViewMatrix[3][0] = -glm::dot(u, position);
    pViewMatrix[3][1] = -glm::dot(v, position);
    pViewMatrix[3][2] = -glm::dot(w, position);
  }

}
