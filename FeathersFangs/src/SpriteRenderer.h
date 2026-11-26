#pragma once
#include <glm/glm.hpp>
#include <vector>

class Shader;
class Texture;

class SpriteRenderer
{
public:
    explicit SpriteRenderer(Shader& shader);
    ~SpriteRenderer();
    SpriteRenderer(const SpriteRenderer&) = delete;
    SpriteRenderer& operator=(const SpriteRenderer&) = delete;

    // position / size w pikselach, rotation w stopniach, color = tint (RGBA).
    void DrawBatch(const Texture& texture, const std::vector<glm::vec4>& rectangles, const glm::vec4& color) const;

    void Draw(const Texture& texture,
              const glm::vec2& position,
              const glm::vec2& size,
              float rotation = 0.0f,
              const glm::vec4& color = glm::vec4(1.0f)) const;

private:
    void InitQuad();

    Shader& m_shader;
    unsigned int m_vao = 0;
    unsigned int m_vbo = 0;
    unsigned int m_batchVao = 0;
    unsigned int m_batchVbo = 0;
};
