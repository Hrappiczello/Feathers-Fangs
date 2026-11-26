#include "Projectile.h"

Projectile::Projectile(std::shared_ptr<Texture> texture, const glm::vec2& pos, float projectileDamage,
                       bool slowEffect, bool stunEffect, bool knockbackEffect, bool piercingEffect)
    : GameObject(std::move(texture), pos, glm::vec2(28.0f), glm::vec2(420.0f, 0.0f)),
      damage(projectileDamage), slows(slowEffect), stuns(stunEffect),
      knockback(knockbackEffect), piercing(piercingEffect)
{
    hitboxScale = 1.0f;
}
