#ifndef SVKE_CAMERA_HPP
#define SVKE_CAMERA_HPP

#include "defines.hpp"
#include "pch.hpp"

namespace svke {
  class Camera {
   public:
    void UseOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void UsePerspectiveProjection(float fov_y, float aspect_ratio, float near, float far);

    void SetViewDirection(const glm::vec3& position,
                          const glm::vec3& direction,
                          const glm::vec3& up_direction = glm::vec3 {0.0f, -1.0f, 0.0f});
    void SetViewTarget(const glm::vec3& position,
                       const glm::vec3& target,
                       const glm::vec3& up_direction = glm::vec3 {0.0f, -1.0f, 0.0f});
    void SetViewYXZ(const glm::vec3& position, const glm::vec3& rotation);

    const glm::mat4& getProjectionMatrix() const { return pProjectionMatrix; }
    const glm::mat4& getViewMatrix() const { return pViewMatrix; }

   private:
    glm::mat4 pProjectionMatrix {1.0f};
    glm::mat4 pViewMatrix {1.0f};
  };
}

#endif
