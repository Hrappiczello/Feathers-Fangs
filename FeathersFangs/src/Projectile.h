#pragma once
#include "GameObject.h"

class Projectile : public GameObject
{
public:
    Projectile(std::shared_ptr<Texture> texture, const glm::vec2& position, float damage = 20.0f,
               bool slows = false, bool stuns = false, bool knockback = false, bool piercing = false);

    float damage = 20.0f;
    bool slows = false;
    bool stuns = false;
    bool knockback = false;
    bool piercing = false;
};
