#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "Texture.h"

class SpriteRenderer;

// Klasa bazowa dla wszystkich obiektow gry (Bird, Fox, Projectile...).
class GameObject
{
public:
    GameObject() = default;
    GameObject(std::shared_ptr<Texture> texture,
               const glm::vec2& position,
               const glm::vec2& size,
               const glm::vec2& velocity = glm::vec2(0.0f));
    virtual ~GameObject() = default;

    virtual void Update(float dt);
    virtual void Draw(const SpriteRenderer& renderer) const;

    // Kolizja AABB (prostokaty wyrownane do osi). Hitbox to pomniejszony sprite,
    // zeby postacie nie "zahaczaly" o siebie samymi przezroczystymi krawedziami.
    bool Overlaps(const GameObject& other) const;

    glm::vec2 position{ 0.0f };
    glm::vec2 size{ 64.0f };
    glm::vec2 velocity{ 0.0f };
    float rotation = 0.0f;                 // stopnie
    glm::vec4 color{ 1.0f };               // tint
    float hitboxScale = 0.6f;              // 1.0 = caly sprite
    bool active = true;                    // false = do usuniecia

protected:
    std::shared_ptr<Texture> m_texture;
};
