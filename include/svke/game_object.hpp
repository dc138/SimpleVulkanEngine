#ifndef SVKE_GAME_OBJECT_HPP
#define SVKE_GAME_OBJECT_HPP

#include "defines.hpp"
#include "model.hpp"
#include "pch.hpp"

namespace svke {
  struct TransformComponent {
    glm::vec3 translation {};
    glm::vec3 scale {};
    glm::vec3 rotation {};

    // Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
    // Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
    // https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
    glm::mat4 matrix() {
      const float c3 = glm::cos(rotation.z);
      const float s3 = glm::sin(rotation.z);
      const float c2 = glm::cos(rotation.x);
      const float s2 = glm::sin(rotation.x);
      const float c1 = glm::cos(rotation.y);
      const float s1 = glm::sin(rotation.y);

      return {{
                  scale.x * (c1 * c3 + s1 * s2 * s3),
                  scale.x * (c2 * s3),
                  scale.x * (c1 * s2 * s3 - c3 * s1),
                  0.0f,
              },
              {
                  scale.y * (c3 * s1 * s2 - c1 * s3),
                  scale.y * (c2 * c3),
                  scale.y * (c1 * c3 * s2 + s1 * s3),
                  0.0f,
              },
              {
                  scale.z * (c2 * s1),
                  scale.z * (-s2),
                  scale.z * (c1 * c2),
                  0.0f,
              },
              {translation.x, translation.y, translation.z, 1.0f}};
    }
  };

  class GameObject {
   public:
    using id_t = uint32_t;

    static GameObject CreateGameObject() {
      static id_t current_id = 0;
      return GameObject {current_id++};
    }

   public:
    id_t getId() const { return pId; }

   public:
    GameObject(const GameObject &) = delete;
    GameObject &operator=(const GameObject &) = delete;
    GameObject(GameObject &&)                 = default;
    GameObject &operator=(GameObject &&) = default;

   public:
    std::shared_ptr<Model> ObjectModel {};
    TransformComponent     Transform {};

   private:
    GameObject(id_t id) : pId {id} {};

    id_t pId {};
  };
}  // namespace svke

#endif
