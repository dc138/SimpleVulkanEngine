#ifndef SVKE_CAMERA_HPP
#define SVKE_CAMERA_HPP

#include "defines.hpp"
#include "pch.hpp"

namespace svke {
  class Camera {
   public:
    void UseOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
    void UsePerspectiveProjection(float fov_y, float aspect_ratio, float near, float far);

    const glm::mat4& getProjectionMatrix() const { return pProjectionMatrix; }

   private:
    glm::mat4 pProjectionMatrix {1.0f};
  };
}

#endif
