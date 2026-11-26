#pragma once
#include <memory>
#include <array>
#include <random>
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Bird.h"
#include "Fox.h"
#include "GameObject.h"
#include "Grid.h"
#include "Projectile.h"
#include "Shader.h"
#include "SpriteRenderer.h"
#include "Texture.h"

struct GLFWwindow;

class Game
{
public:
    static constexpr int WIDTH = 1280;
    static constexpr int HEIGHT = 720;

    Game() = default;
    ~Game();
    bool Init();
    void Run();

private:
    bool LoadResources();
    void ProcessInput();
    void Update(float dt);
    void Render();
    void Shutdown();
    glm::vec2 GetMousePos() const;
    void HandlePlacement();
    void SpawnFox();
    void SpawnSandboxFox(FoxType type);
    void SpawnSandboxGroup();
    void StartSandbox();
    void ResetSandboxField();
    bool FoxInFront(const Bird& bird) const;
    void ShootFrom(const Bird& bird);
    void ResolveCollisions(float dt);
    void Cleanup();
    void ResetMatch();
    void LoadProgress();
    void SaveProgress();
    void StartLevel(int level);
    void CompleteLevel();
    void UpdateWindowTitle();
    void SpawnSkyEgg();
    void SpawnParticles(const glm::vec2& position, const glm::vec4& color, int count);
    void DrawGameText(const std::string& text, float x, float y, float scale, const glm::vec4& color);
    void DrawHud();
    void DrawMenu();
    void DrawHelpMenu();
    void PlayEffect(const char* file);

    GLFWwindow* m_window = nullptr;
    Shader m_spriteShader;
    std::unique_ptr<SpriteRenderer> m_renderer;
    std::shared_ptr<Texture> m_texWhite;
    std::array<std::shared_ptr<Texture>, 9> m_texBirds;
    std::array<std::shared_ptr<Texture>, 6> m_texFoxes;
    std::shared_ptr<Texture> m_texGrain;

    Grid m_grid;
    std::vector<std::unique_ptr<Bird>> m_birds;
    std::vector<std::unique_ptr<Fox>> m_foxes;
    std::vector<std::unique_ptr<Projectile>> m_projectiles;
    std::vector<std::unique_ptr<GameObject>> m_skyEggs;

    struct Particle
    {
        glm::vec2 position{ 0.0f };
        glm::vec2 velocity{ 0.0f };
        glm::vec4 color{ 1.0f };
        float life = 0.0f;
        float maxLife = 1.0f;
        float size = 5.0f;
    };
    std::vector<Particle> m_particles;

    int m_hoverRow = -1;
    int m_hoverCol = -1;
    bool m_mouseWasDown = false;
    int m_eggs = 150;
    int m_savedCampaignEggs = 150;
    int m_selectedBird = 0;
    int m_currentLevel = 1;
    int m_unlockedLevel = 1;
    int m_foxesSpawned = 0;
    int m_foxesToSpawn = 6;
    int m_totalCompleted = 0;
    bool m_campaignWon = false;
    bool m_inMenu = true;
    bool m_sandbox = false;
    bool m_showHelp = false;
    int m_menuSelection = 0;
    bool m_paused = false;
    bool m_pauseKeyWasDown = false;
    bool m_escapeKeyWasDown = false;
    bool m_enterKeyWasDown = false;
    bool m_menuUpWasDown = false;
    bool m_menuDownWasDown = false;
    std::array<bool, 9> m_sandboxKeyWasDown{};
    float m_spawnTimer = 3.0f;
    float m_skyEggTimer = 5.0f;
    float m_levelMessageTimer = 0.0f;
    std::mt19937 m_rng{ std::random_device{}() };
};
