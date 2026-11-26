#pragma once
#include "GameObject.h"

enum class BirdType
{
    Chicken = 0,
    LayingHen,
    WarriorRooster,
    ArmoredTurkey,
    MudDuck,
    Owl,
    BomberPigeon,
    WarGoose,
    Emu
};

class Bird : public GameObject
{
public:
    Bird(std::shared_ptr<Texture> texture, int row, int col, BirdType type = BirdType::Chicken);

    void Update(float dt) override;
    bool ReadyToShoot() const { return m_cooldown <= 0.0f; }
    void ResetCooldown() { m_cooldown = fireInterval; }
    void TakeDamage(float amount);

    int row;
    int col;
    BirdType type;
    float health = 150.0f;
    float fireInterval = 1.4f;
    float attackDamage = 20.0f;
    float eggTimer = 8.0f;
    int cost = 100;
    bool IsRanged() const;
    bool IsMelee() const;

private:
    float m_cooldown = 0.5f;
};
