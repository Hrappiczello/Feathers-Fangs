#pragma once
#include <string>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader() = default;
    ~Shader() { Release(); }
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    // Wczytuje, kompiluje i linkuje shadery. Zwraca false przy błędzie.
    bool Load(const std::string& vertPath, const std::string& fragPath);
    void Release();
    void Use() const;

    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;
    void SetVec2(const std::string& name, const glm::vec2& value) const;
    void SetVec4(const std::string& name, const glm::vec4& value) const;
    void SetMat4(const std::string& name, const glm::mat4& value) const;

private:
    unsigned int m_id = 0;
};
