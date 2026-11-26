#include "Grid.h"
#include "SpriteRenderer.h"
#include "Texture.h"

glm::vec2 Grid::CellPosition(int row, int col)
{
    return glm::vec2(ORIGIN_X + col * CELL_W, ORIGIN_Y + row * CELL_H);
}

bool Grid::PixelToCell(const glm::vec2& pixel, int& row, int& col)
{
    const float lx = pixel.x - ORIGIN_X;
    const float ly = pixel.y - ORIGIN_Y;
    if (lx < 0.0f || ly < 0.0f)
        return false;

    col = static_cast<int>(lx / CELL_W);
    row = static_cast<int>(ly / CELL_H);
    return row < ROWS && col < COLS;
}

void Grid::Clear()
{
    for (int r = 0; r < ROWS; ++r)
        for (int c = 0; c < COLS; ++c)
            m_occupied[r][c] = false;
}

void Grid::Draw(const SpriteRenderer& renderer, const Texture& white, int hoverRow, int hoverCol) const
{
    const glm::vec4 light(0.40f, 0.70f, 0.30f, 1.0f);
    const glm::vec4 dark(0.34f, 0.62f, 0.26f, 1.0f);
    const glm::vec4 hoverFree(0.65f, 0.90f, 0.45f, 1.0f);
    const glm::vec4 hoverBusy(0.80f, 0.35f, 0.30f, 1.0f);

    for (int r = 0; r < ROWS; ++r)
    {
        for (int c = 0; c < COLS; ++c)
        {
            glm::vec4 color = ((r + c) % 2 == 0) ? light : dark;
            if (r == hoverRow && c == hoverCol)
                color = m_occupied[r][c] ? hoverBusy : hoverFree;

            renderer.Draw(white, CellPosition(r, c), glm::vec2(CELL_W, CELL_H), 0.0f, color);
        }
    }
}
