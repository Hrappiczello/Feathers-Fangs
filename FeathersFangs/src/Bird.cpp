#include "Bird.h"
#include <utility>
#include "Grid.h"

static constexpr float BIRD_SIZE = 100.0f;

Bird::Bird(std::shared_ptr<Texture> texture, int r, int c, BirdType birdType)
    : GameObject(std::move(texture), glm::vec2(0.0f), glm::vec2(BIRD_SIZE)), row(r), col(c), type(birdType)
{
    position = Grid::CellPosition(r, c) +
               glm::vec2((Grid::CELL_W - BIRD_SIZE) * 0.5f, (Grid::CELL_H - BIRD_SIZE) * 0.5f);

    switch (type)
    {
    case BirdType::Chicken:
        cost = 100; health = 150.0f; fireInterval = 1.4f; attackDamage = 20.0f;
        color = glm::vec4(1.0f); break;
    case BirdType::LayingHen:
        cost = 50; health = 110.0f; fireInterval = 99.0f; attackDamage = 0.0f;
        color = glm::vec4(1.0f, 0.85f, 0.35f, 1.0f); break;
    case BirdType::WarriorRooster:
        cost = 175; health = 190.0f; fireInterval = 0.8f; attackDamage = 55.0f;
        color = glm::vec4(1.0f, 0.35f, 0.25f, 1.0f); break;
    case BirdType::ArmoredTurkey:
        cost = 125; health = 500.0f; fireInterval = 99.0f; attackDamage = 0.0f;
        color = glm::vec4(0.55f, 0.75f, 1.0f, 1.0f); break;
    case BirdType::MudDuck:
        cost = 150; health = 140.0f; fireInterval = 2.0f; attackDamage = 28.0f;
        color = glm::vec4(0.45f, 0.8f, 0.95f, 1.0f); break;
    case BirdType::Owl:
        cost = 200; health = 130.0f; fireInterval = 2.3f; attackDamage = 18.0f;
        color = glm::vec4(0.7f, 0.65f, 1.0f, 1.0f); break;
    case BirdType::BomberPigeon:
        cost = 125; health = 1.0f; fireInterval = 99.0f; attackDamage = 180.0f;
        color = glm::vec4(0.8f, 0.8f, 0.8f, 1.0f); break;
    case BirdType::WarGoose:
        cost = 225; health = 220.0f; fireInterval = 1.5f; attackDamage = 35.0f;
        color = glm::vec4(0.9f, 0.95f, 0.65f, 1.0f); break;
    case BirdType::Emu:
        cost = 350; health = 260.0f; fireInterval = 2.8f; attackDamage = 100.0f;
        color = glm::vec4(0.75f, 0.55f, 0.35f, 1.0f); break;
    }
    m_cooldown = 0.5f;
}

void Bird::Update(float dt)
{
    GameObject::Update(dt);
    if (m_cooldown > 0.0f) m_cooldown -= dt;
    if (type == BirdType::LayingHen && eggTimer > 0.0f) eggTimer -= dt;
}

void Bird::TakeDamage(float amount)
{
    health -= amount;
    if (health <= 0.0f) active = false;
}

bool Bird::IsRanged() const
{
    return type == BirdType::Chicken || type == BirdType::MudDuck || type == BirdType::Owl ||
           type == BirdType::WarGoose || type == BirdType::Emu;
}

bool Bird::IsMelee() const
{
    return type == BirdType::WarriorRooster || type == BirdType::BomberPigeon;
}
