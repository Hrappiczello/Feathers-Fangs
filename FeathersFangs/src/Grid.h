#pragma once
#include <glm/glm.hpp>

class SpriteRenderer;
class Texture;

// Pole walki 5x9. Pozycje w pikselach, (0,0) = lewy gorny rog okna.
class Grid
{
public:
    static constexpr int ROWS = 5;
    static constexpr int COLS = 9;
    static constexpr float CELL_W = 120.0f;
    static constexpr float CELL_H = 108.0f;
    static constexpr float ORIGIN_X = 100.0f;  // lewy gorny rog siatki
    static constexpr float ORIGIN_Y = 174.0f;

    // Lewy gorny rog komorki (row, col)
    static glm::vec2 CellPosition(int row, int col);

    // Zamienia pozycje myszy na komorke. Zwraca false, jesli kursor jest poza siatka.
    static bool PixelToCell(const glm::vec2& pixel, int& row, int& col);

    bool IsOccupied(int row, int col) const { return m_occupied[row][col]; }
    void SetOccupied(int row, int col, bool value) { m_occupied[row][col] = value; }
    void Clear();

    // hoverRow/hoverCol = -1, gdy kursor nie jest nad zadna komorka
    void Draw(const SpriteRenderer& renderer, const Texture& white, int hoverRow, int hoverCol) const;

private:
    bool m_occupied[ROWS][COLS] = {};
};
