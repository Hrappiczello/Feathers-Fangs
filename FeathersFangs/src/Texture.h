#pragma once
#include <string>

class Texture
{
public:
    Texture() = default;
    ~Texture() { Release(); }
    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

    // Wczytuje PNG (z kanalem alfa) przez stb_image.
    bool Load(const std::string& path);

    // Tworzy teksture 1x1 o jednolitym kolorze (przyda sie do prostokatow, siatki, HUD-u).
    void CreateSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 255);

    void Bind(unsigned int unit = 0) const;
    void Release();

    int Width() const { return m_width; }
    int Height() const { return m_height; }

private:
    void Upload(const unsigned char* data, int width, int height);

    unsigned int m_id = 0;
    int m_width = 0;
    int m_height = 0;
};
