#ifndef SVKE_GAME_OBJECT_HPP
#define SVKE_GAME_OBJECT_HPP

#include "defines.hpp"
#include "model.hpp"
#include "pch.hpp"

namespace svke {
  struct Transform2d {
    glm::vec2 translation {0.0f, 0.0f};
    glm::vec2 scale {1.0f, 1.0f};
    float     rotation = 0.0f;

    glm::mat2 getMat2() {
      const float s = std::sin(rotation);
      const float c = std::cos(rotation);

      glm::mat2 rotation_matrix {{c, s}, {-s, c}};
      glm::mat2 scale_matrix {{scale.x, 0.0f}, {0.0f, scale.y}};

      return rotation_matrix * scale_matrix;
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
    Transform2d            ObjectTransform2d {};

   private:
    GameObject(id_t id) : pId {id} {};

    id_t pId {};
  };
}  // namespace svke

#endif
