#pragma once
#include "GameObject.h"

enum class FoxType
{
    Common = 0,
    Sprint,
    Armored,
    Jumper,
    Miner,
    Alpha
};

class Fox : public GameObject
{
public:
    Fox(std::shared_ptr<Texture> texture, int row, float spawnX, FoxType type = FoxType::Common);

    void Update(float dt) override;
    void TakeDamage(float amount);
    void ApplySlow(float seconds);
    void ApplyStun(float seconds);

    int row;
    FoxType type;
    float health = 100.0f;
    float maxHealth = 100.0f;
    float speed = 25.0f;
    float damagePerSecond = 25.0f;
    float armorHealth = 0.0f;
    bool eating = false;
    bool canBeHit = true;
    bool hasJumped = false;
    bool spawnedMinions = false;

private:
    float m_flash = 0.0f;
    float m_slowTimer = 0.0f;
    float m_stunTimer = 0.0f;
    float m_undergroundTimer = 0.0f;
};
