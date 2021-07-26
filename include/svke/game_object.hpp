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

    glm::mat4 matrix() {
      glm::mat4 transform {1.0f};

      transform = glm::translate(transform, translation);
      transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.0f});
      transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.0f});
      transform = glm::rotate(transform, rotation.z, {0.0f, 0.0f, 1.0f});
      transform = glm::scale(transform, scale);

      return transform;
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
