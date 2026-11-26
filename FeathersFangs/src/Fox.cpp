#include "Fox.h"
#include <utility>
#include "Grid.h"
#include <algorithm>

static constexpr float FOX_SIZE = 100.0f;

Fox::Fox(std::shared_ptr<Texture> texture, int r, float spawnX, FoxType foxType)
    : GameObject(std::move(texture), glm::vec2(0.0f), glm::vec2(FOX_SIZE)), row(r), type(foxType)
{
    position = glm::vec2(spawnX, Grid::CellPosition(r, 0).y + (Grid::CELL_H - FOX_SIZE) * 0.5f);
    switch (type)
    {
    case FoxType::Common: health = 100; speed = 25; damagePerSecond = 25; break;
    case FoxType::Sprint: health = 55; speed = 58; damagePerSecond = 18; color = glm::vec4(1.0f, 0.65f, 0.35f, 1.0f); break;
    case FoxType::Armored: health = 130; speed = 17; damagePerSecond = 28; armorHealth = 100; color = glm::vec4(0.65f, 0.7f, 0.78f, 1.0f); break;
    case FoxType::Jumper: health = 90; speed = 32; damagePerSecond = 22; color = glm::vec4(0.8f, 0.55f, 0.25f, 1.0f); break;
    case FoxType::Miner: health = 85; speed = 30; damagePerSecond = 20; m_undergroundTimer = 2.5f; color = glm::vec4(0.45f, 0.35f, 0.25f, 1.0f); break;
    case FoxType::Alpha: health = 650; speed = 15; damagePerSecond = 55; size = glm::vec2(112.0f); color = glm::vec4(0.85f, 0.25f, 0.25f, 1.0f); break;
    }
    maxHealth = health;
}

void Fox::Update(float dt)
{
    if (m_slowTimer > 0.0f) m_slowTimer -= dt;
    if (m_stunTimer > 0.0f) m_stunTimer -= dt;
    if (m_flash > 0.0f) m_flash -= dt;
    if (type == FoxType::Miner && m_undergroundTimer > 0.0f)
    {
        m_undergroundTimer -= dt;
        canBeHit = false;
        color.a = 0.35f;
    }
    else
    {
        canBeHit = true;
        color.a = 1.0f;
    }

    float multiplier = (m_slowTimer > 0.0f) ? 0.45f : 1.0f;
    velocity.x = (eating || m_stunTimer > 0.0f) ? 0.0f : -speed * multiplier;
    GameObject::Update(dt);
    if (m_flash > 0.0f) color.r = 1.0f;
}

void Fox::TakeDamage(float amount)
{
    if (!canBeHit) return;
    if (armorHealth > 0.0f)
    {
        float absorbed = std::min(armorHealth, amount);
        armorHealth -= absorbed;
        amount -= absorbed;
    }
    health -= amount;
    m_flash = 0.12f;
    if (health <= 0.0f) active = false;
}

void Fox::ApplySlow(float seconds) { m_slowTimer = std::max(m_slowTimer, seconds); }
void Fox::ApplyStun(float seconds) { m_stunTimer = std::max(m_stunTimer, seconds); }
