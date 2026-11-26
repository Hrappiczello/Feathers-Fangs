#include "GameObject.h"
#include "SpriteRenderer.h"

GameObject::GameObject(std::shared_ptr<Texture> texture,
                       const glm::vec2& pos,
                       const glm::vec2& sz,
                       const glm::vec2& vel)
    : position(pos), size(sz), velocity(vel), m_texture(std::move(texture))
{
}

void GameObject::Update(float dt)
{
    position += velocity * dt;
}

void GameObject::Draw(const SpriteRenderer& renderer) const
{
    if (m_texture)
        renderer.Draw(*m_texture, position, size, rotation, color);
}

bool GameObject::Overlaps(const GameObject& other) const
{
    const glm::vec2 aSize = size * hitboxScale;
    const glm::vec2 aPos = position + (size - aSize) * 0.5f;
    const glm::vec2 bSize = other.size * other.hitboxScale;
    const glm::vec2 bPos = other.position + (other.size - bSize) * 0.5f;

    return aPos.x < bPos.x + bSize.x && aPos.x + aSize.x > bPos.x &&
           aPos.y < bPos.y + bSize.y && aPos.y + aSize.y > bPos.y;
}
