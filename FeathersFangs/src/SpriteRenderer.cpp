#include "SpriteRenderer.h"
#include "Shader.h"
#include "Texture.h"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

SpriteRenderer::SpriteRenderer(Shader& shader) : m_shader(shader)
{
    InitQuad();
}

SpriteRenderer::~SpriteRenderer()
{
    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteVertexArrays(1, &m_batchVao);
    glDeleteBuffers(1, &m_batchVbo);
}

void SpriteRenderer::InitQuad()
{
    // Jednostkowy kwadrat (0..1) z UV; skalujemy go macierza model.
    // Uklad: x, y, u, v. Poczatek ukladu (0,0) = lewy gorny rog.
    const float vertices[] = {
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f,

        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindVertexArray(m_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Osobny bufor dla tekstu. DrawBatch nie moze nadpisywac
    // wierzcholkow jednostkowego kwadratu uzywanego przez Draw().
    glGenVertexArrays(1, &m_batchVao);
    glGenBuffers(1, &m_batchVbo);
    glBindVertexArray(m_batchVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_batchVbo);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SpriteRenderer::Draw(const Texture& texture,
                          const glm::vec2& position,
                          const glm::vec2& size,
                          float rotation,
                          const glm::vec4& color) const
{
    m_shader.Use();

    // Kolejnosc: skala -> obrot wokol srodka -> przesuniecie.
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::translate(model, glm::vec3(0.5f * size.x, 0.5f * size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x, -0.5f * size.y, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    m_shader.SetMat4("model", model);
    m_shader.SetVec4("spriteColor", color);

    texture.Bind(0);

    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void SpriteRenderer::DrawBatch(const Texture& texture,
                               const std::vector<glm::vec4>& rectangles,
                               const glm::vec4& color) const
{
    if (rectangles.empty()) return;

    std::vector<float> vertices;
    vertices.reserve(rectangles.size() * 24);
    for (const glm::vec4& rect : rectangles)
    {
        float x = rect.x, y = rect.y, w = rect.z, h = rect.w;
        const float quad[] = {
            x,     y + h, 0.0f, 1.0f,
            x + w, y,     1.0f, 0.0f,
            x,     y,     0.0f, 0.0f,
            x,     y + h, 0.0f, 1.0f,
            x + w, y + h, 1.0f, 1.0f,
            x + w, y,     1.0f, 0.0f
        };
        vertices.insert(vertices.end(), quad, quad + 24);
    }

    m_shader.Use();
    m_shader.SetMat4("model", glm::mat4(1.0f));
    m_shader.SetVec4("spriteColor", color);
    texture.Bind(0);

    glBindVertexArray(m_batchVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_batchVbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size() / 4));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
