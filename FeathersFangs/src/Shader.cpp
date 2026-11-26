#include "Shader.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

static bool ReadFile(const std::string& path, std::string& out)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cerr << "[Shader] Nie mozna otworzyc pliku: " << path << "\n";
        return false;
    }
    std::stringstream ss;
    ss << file.rdbuf();
    out = ss.str();
    return true;
}

static unsigned int Compile(GLenum type, const std::string& src, const std::string& name)
{
    unsigned int shader = glCreateShader(type);
    const char* code = src.c_str();
    glShaderSource(shader, 1, &code, nullptr);
    glCompileShader(shader);

    int ok = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, log);
        std::cerr << "[Shader] Blad kompilacji (" << name << "):\n" << log << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

bool Shader::Load(const std::string& vertPath, const std::string& fragPath)
{
    std::string vertSrc, fragSrc;
    if (!ReadFile(vertPath, vertSrc) || !ReadFile(fragPath, fragSrc))
        return false;

    unsigned int vs = Compile(GL_VERTEX_SHADER, vertSrc, vertPath);
    unsigned int fs = Compile(GL_FRAGMENT_SHADER, fragSrc, fragPath);
    if (!vs || !fs)
    {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return false;
    }

    Release();
    m_id = glCreateProgram();
    glAttachShader(m_id, vs);
    glAttachShader(m_id, fs);
    glLinkProgram(m_id);

    glDeleteShader(vs);
    glDeleteShader(fs);

    int ok = 0;
    glGetProgramiv(m_id, GL_LINK_STATUS, &ok);
    if (!ok)
    {
        char log[1024];
        glGetProgramInfoLog(m_id, 1024, nullptr, log);
        std::cerr << "[Shader] Blad linkowania:\n" << log << "\n";
        Release();
        return false;
    }
    return true;
}

void Shader::Release()
{
    if (m_id)
    {
        glDeleteProgram(m_id);
        m_id = 0;
    }
}

void Shader::Use() const { glUseProgram(m_id); }

void Shader::SetInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::SetFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(m_id, name.c_str()), value);
}

void Shader::SetVec2(const std::string& name, const glm::vec2& value) const
{
    glUniform2fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetVec4(const std::string& name, const glm::vec4& value) const
{
    glUniform4fv(glGetUniformLocation(m_id, name.c_str()), 1, glm::value_ptr(value));
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
