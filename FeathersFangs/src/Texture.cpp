#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "Texture.h"
#include <glad/glad.h>
#include <iostream>

void Texture::Upload(const unsigned char* data, int width, int height)
{
    Release();
    m_width = width;
    m_height = height;

    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

bool Texture::Load(const std::string& path)
{
    int w = 0, h = 0, channels = 0;
    // Wymuszamy 4 kanaly (RGBA), zeby alfa zawsze byla dostepna.
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data)
    {
        std::cerr << "[Texture] Nie mozna wczytac: " << path << " (" << stbi_failure_reason() << ")\n";
        return false;
    }
    Upload(data, w, h);
    stbi_image_free(data);
    return true;
}

void Texture::CreateSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    unsigned char pixel[4] = { r, g, b, a };
    Upload(pixel, 1, 1);
}

void Texture::Bind(unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_id);
}

void Texture::Release()
{
    if (m_id)
    {
        glDeleteTextures(1, &m_id);
        m_id = 0;
    }
}
