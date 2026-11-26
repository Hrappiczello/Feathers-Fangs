#include "Game.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <array>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <cstdint>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#include <mmsystem.h>
#endif

static constexpr float FIRST_FOX_DELAY = 3.0f;
static constexpr float SKY_EGG_INTERVAL = 8.0f;

static const std::array<BirdType, 9> BIRD_TYPES = {
    BirdType::Chicken, BirdType::LayingHen, BirdType::WarriorRooster,
    BirdType::ArmoredTurkey, BirdType::MudDuck, BirdType::Owl,
    BirdType::BomberPigeon, BirdType::WarGoose, BirdType::Emu
};

static const std::array<const char*, 9> BIRD_NAMES = {
    "Kura zwyczajna", "Kura nioska", "Kogut wojownik", "Indyk pancerny",
    "Kaczka blotna", "Sowa straznik", "Golab bombowiec", "Ges bojowa", "Strus Emu"
};


static std::array<unsigned char, 5> Glyph(char c)
{
    switch (c)
    {
    case 'A': return {14, 17, 31, 17, 17}; case 'B': return {30, 17, 30, 17, 30};
    case 'C': return {14, 17, 16, 17, 14}; case 'D': return {30, 17, 17, 17, 30};
    case 'E': return {31, 16, 30, 16, 31}; case 'F': return {31, 16, 30, 16, 16};
    case 'G': return {14, 16, 23, 17, 15}; case 'H': return {17, 17, 31, 17, 17};
    case 'I': return {31, 4, 4, 4, 31}; case 'J': return {7, 2, 2, 18, 12};
    case 'K': return {17, 18, 28, 18, 17}; case 'L': return {16, 16, 16, 16, 31};
    case 'M': return {17, 27, 21, 17, 17}; case 'N': return {17, 25, 21, 19, 17};
    case 'O': return {14, 17, 17, 17, 14}; case 'P': return {30, 17, 30, 16, 16};
    case 'Q': return {14, 17, 17, 21, 11}; case 'R': return {30, 17, 30, 18, 17};
    case 'S': return {15, 16, 14, 1, 30}; case 'T': return {31, 4, 4, 4, 4};
    case 'U': return {17, 17, 17, 17, 14}; case 'V': return {17, 17, 17, 10, 4};
    case 'W': return {17, 17, 21, 27, 17}; case 'X': return {17, 10, 4, 10, 17};
    case 'Y': return {17, 10, 4, 4, 4}; case 'Z': return {31, 2, 4, 8, 31};
    case '0': return {14, 17, 19, 17, 14}; case '1': return {4, 12, 4, 4, 14};
    case '2': return {14, 17, 2, 4, 31}; case '3': return {30, 1, 6, 1, 30};
    case '4': return {2, 6, 10, 31, 2}; case '5': return {31, 16, 30, 1, 30};
    case '6': return {14, 16, 30, 17, 14}; case '7': return {31, 1, 2, 4, 4};
    case '8': return {14, 17, 14, 17, 14}; case '9': return {14, 17, 15, 1, 14};
    case ':': return {0, 4, 0, 4, 0}; case '/': return {1, 2, 4, 8, 16};
    case '-': return {0, 0, 31, 0, 0}; case '.': return {0, 0, 0, 0, 4};
    case '!': return {4, 4, 4, 0, 4}; case '&': return {10, 21, 10, 21, 10};
    default: return {0, 0, 0, 0, 0};
    }
}

static void FramebufferSizeCallback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

template <typename T>
static void RemoveInactive(std::vector<std::unique_ptr<T>>& v)
{
    v.erase(std::remove_if(v.begin(), v.end(),
                           [](const std::unique_ptr<T>& p) { return !p->active; }), v.end());
}

static int ReadJsonInt(const std::string& data, const std::string& key, int fallback)
{
    const std::string token = "\"" + key + "\"";
    size_t pos = data.find(token);
    if (pos == std::string::npos) return fallback;
    pos = data.find(':', pos + token.size());
    if (pos == std::string::npos) return fallback;
    try { return std::stoi(data.substr(pos + 1)); }
    catch (...) { return fallback; }
}

Game::~Game() { Shutdown(); }

bool Game::Init()
{
    if (!glfwInit())
    {
        std::cerr << "[Game] Nie udalo sie zainicjalizowac GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Feathers & Fangs: Chickens vs Foxes", nullptr, nullptr);
    if (!m_window)
    {
        std::cerr << "[Game] Nie udalo sie utworzyc okna\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    glfwSetFramebufferSizeCallback(m_window, FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "[Game] Nie udalo sie zaladowac GLAD\n";
        return false;
    }

    int fbw, fbh;
    glfwGetFramebufferSize(m_window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (!LoadResources()) return false;

    LoadProgress();
    bool alreadyWon = m_totalCompleted >= 10;
    StartLevel(m_currentLevel);
    if (alreadyWon) m_campaignWon = true;
    m_inMenu = true;
    UpdateWindowTitle();
    std::cout << "Sterowanie: ENTER start, 1-9 wybor ptaka, LPM postaw/zbierz jajko, P pauza, ESC menu\n";
    return true;
}

bool Game::LoadResources()
{
    if (!m_spriteShader.Load("shaders/sprite.vert", "shaders/sprite.frag")) return false;

    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, (float)HEIGHT, 0.0f, -1.0f, 1.0f);
    m_spriteShader.Use();
    m_spriteShader.SetInt("image", 0);
    m_spriteShader.SetMat4("projection", projection);
    m_renderer = std::make_unique<SpriteRenderer>(m_spriteShader);

    m_texWhite = std::make_shared<Texture>();
    m_texWhite->CreateSolid(255, 255, 255);
    for (int i = 0; i < static_cast<int>(m_texBirds.size()); ++i)
    {
        m_texBirds[i] = std::make_shared<Texture>();
        if (!m_texBirds[i]->Load(("textures/bird_" + std::to_string(i) + ".png").c_str())) return false;
    }
    for (int i = 0; i < static_cast<int>(m_texFoxes.size()); ++i)
    {
        m_texFoxes[i] = std::make_shared<Texture>();
        if (!m_texFoxes[i]->Load(("textures/fox_" + std::to_string(i) + ".png").c_str())) return false;
    }
    m_texGrain = std::make_shared<Texture>();
    if (!m_texGrain->Load("textures/grain.png")) return false;
    return true;
}

void Game::Run()
{
    double last = glfwGetTime();
    while (!glfwWindowShouldClose(m_window))
    {
        double now = glfwGetTime();
        float dt = (float)(now - last);
        last = now;
        dt = std::min(dt, 0.05f);
        glfwPollEvents();
        ProcessInput();
        Update(dt);
        Render();
    }
}

void Game::ProcessInput()
{
    const bool pauseDown = glfwGetKey(m_window, GLFW_KEY_P) == GLFW_PRESS;
    const bool escapeDown = glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS;
    const bool enterDown = glfwGetKey(m_window, GLFW_KEY_ENTER) == GLFW_PRESS ||
                           glfwGetKey(m_window, GLFW_KEY_KP_ENTER) == GLFW_PRESS;
    const bool upDown = glfwGetKey(m_window, GLFW_KEY_UP) == GLFW_PRESS ||
                        glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS;
    const bool downDown = glfwGetKey(m_window, GLFW_KEY_DOWN) == GLFW_PRESS ||
                          glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS;
    const bool escapePressed = escapeDown && !m_escapeKeyWasDown;
    const bool enterPressed = enterDown && !m_enterKeyWasDown;
    const bool upPressed = upDown && !m_menuUpWasDown;
    const bool downPressed = downDown && !m_menuDownWasDown;

    m_escapeKeyWasDown = escapeDown;
    m_enterKeyWasDown = enterDown;
    m_menuUpWasDown = upDown;
    m_menuDownWasDown = downDown;

    if (m_inMenu)
    {
        m_pauseKeyWasDown = pauseDown;
        if (m_showHelp)
        {
            if (escapePressed || enterPressed) m_showHelp = false;
            return;
        }

        if (upPressed) m_menuSelection = (m_menuSelection + 3) % 4;
        if (downPressed) m_menuSelection = (m_menuSelection + 1) % 4;
        for (int i = 0; i < 4; ++i)
            if (glfwGetKey(m_window, GLFW_KEY_1 + i) == GLFW_PRESS)
                m_menuSelection = i;

        if (enterPressed)
        {
            if (m_menuSelection == 0)
            {
                if (m_sandbox) m_eggs = m_savedCampaignEggs;
                m_sandbox = false;
                StartLevel(m_currentLevel);
                m_inMenu = false;
                m_paused = false;
                m_mouseWasDown = false;
                PlayEffect("audio/start.wav");
            }
            else if (m_menuSelection == 1)
            {
                StartSandbox();
                m_inMenu = false;
                m_paused = false;
                m_mouseWasDown = false;
                PlayEffect("audio/start.wav");
            }
            else if (m_menuSelection == 2)
            {
                m_showHelp = true;
            }
            else
            {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }
        }

        if (escapePressed) glfwSetWindowShouldClose(m_window, GLFW_TRUE);
        return;
    }

    if (escapePressed)
    {
        m_inMenu = true;
        m_paused = false;
        m_mouseWasDown = false;
        m_pauseKeyWasDown = pauseDown;
        m_menuSelection = m_sandbox ? 1 : 0;
        return;
    }

    if (pauseDown && !m_pauseKeyWasDown) m_paused = !m_paused;
    m_pauseKeyWasDown = pauseDown;

    if (!m_paused)
    {
        for (int i = 0; i < 9; ++i)
            if (glfwGetKey(m_window, GLFW_KEY_1 + i) == GLFW_PRESS)
                m_selectedBird = i;

        if (m_sandbox)
        {
            const int keys[9] = {
                GLFW_KEY_F1, GLFW_KEY_F2, GLFW_KEY_F3,
                GLFW_KEY_F4, GLFW_KEY_F5, GLFW_KEY_F6,
                GLFW_KEY_F7, GLFW_KEY_F8, GLFW_KEY_F9
            };
            bool pressed[9]{};
            for (int i = 0; i < 9; ++i)
            {
                bool down = glfwGetKey(m_window, keys[i]) == GLFW_PRESS;
                pressed[i] = down && !m_sandboxKeyWasDown[i];
                m_sandboxKeyWasDown[i] = down;
            }

            if (pressed[0]) SpawnSandboxFox(FoxType::Common);
            if (pressed[1]) SpawnSandboxFox(FoxType::Sprint);
            if (pressed[2]) SpawnSandboxFox(FoxType::Armored);
            if (pressed[3]) SpawnSandboxFox(FoxType::Jumper);
            if (pressed[4]) SpawnSandboxFox(FoxType::Miner);
            if (pressed[5]) SpawnSandboxFox(FoxType::Alpha);
            if (pressed[6]) SpawnSandboxGroup();
            if (pressed[7])
            {
                m_foxes.clear();
                m_projectiles.clear();
            }
            if (pressed[8]) ResetSandboxField();
        }
    }
}

glm::vec2 Game::GetMousePos() const
{
    double mx, my;
    glfwGetCursorPos(m_window, &mx, &my);
    int ww, wh;
    glfwGetWindowSize(m_window, &ww, &wh);
    if (ww == 0 || wh == 0) return glm::vec2(-1.0f);
    return glm::vec2((float)(mx * WIDTH / ww), (float)(my * HEIGHT / wh));
}

void Game::HandlePlacement()
{
    int row = 0, col = 0;
    if (Grid::PixelToCell(GetMousePos(), row, col))
    {
        m_hoverRow = row;
        m_hoverCol = col;
    }
    else m_hoverRow = m_hoverCol = -1;

    const bool down = glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    const bool clicked = down && !m_mouseWasDown;
    m_mouseWasDown = down;
    if (!clicked) return;

    bool collected = false;
    glm::vec2 mouse = GetMousePos();
    for (auto& egg : m_skyEggs)
    {
        glm::vec2 center = egg->position + egg->size * 0.5f;
        if (egg->active && glm::length(center - mouse) < 42.0f)
        {
            egg->active = false;
            if (!m_sandbox) m_eggs += 25;
            collected = true;
            if (!m_sandbox) SaveProgress();
            PlayEffect("audio/egg.wav");
            SpawnParticles(egg->position + egg->size * 0.5f, glm::vec4(1.0f, 0.82f, 0.18f, 1.0f), 10);
            break;
        }
    }
    if (collected) return;

    if (m_hoverRow >= 0 && !m_grid.IsOccupied(m_hoverRow, m_hoverCol))
    {
        BirdType type = BIRD_TYPES[m_selectedBird];
        Bird preview(m_texBirds[static_cast<int>(type)], m_hoverRow, m_hoverCol, type);
        if (m_sandbox || m_eggs >= preview.cost)
        {
            if (!m_sandbox) m_eggs -= preview.cost;
            m_birds.push_back(std::make_unique<Bird>(m_texBirds[static_cast<int>(type)], m_hoverRow, m_hoverCol, type));
            m_grid.SetOccupied(m_hoverRow, m_hoverCol, true);
            if (!m_sandbox) SaveProgress();
            PlayEffect("audio/place.wav");
        }
    }
}

void Game::SpawnFox()
{
    std::uniform_int_distribution<int> rowDist(0, Grid::ROWS - 1);
    FoxType type = FoxType::Common;
    std::uniform_int_distribution<int> chance(0, 99);
    int roll = chance(m_rng);

    if (m_currentLevel >= 3 && roll < 18) type = FoxType::Sprint;
    if (m_currentLevel >= 4 && roll >= 18 && roll < 34) type = FoxType::Armored;
    if (m_currentLevel >= 5 && roll >= 34 && roll < 49) type = FoxType::Jumper;
    if (m_currentLevel >= 6 && roll >= 49 && roll < 64) type = FoxType::Miner;
    if (m_currentLevel >= 7 && m_foxesSpawned == m_foxesToSpawn - 1) type = FoxType::Alpha;

    m_foxes.push_back(std::make_unique<Fox>(m_texFoxes[static_cast<int>(type)], rowDist(m_rng), (float)WIDTH, type));
    ++m_foxesSpawned;
}

void Game::SpawnSandboxFox(FoxType type)
{
    if (!m_sandbox) return;
    std::uniform_int_distribution<int> rowDist(0, Grid::ROWS - 1);
    std::uniform_real_distribution<float> offsetDist(0.0f, 150.0f);
    int row = rowDist(m_rng);
    float spawnX = static_cast<float>(WIDTH) + offsetDist(m_rng);
    m_foxes.push_back(std::make_unique<Fox>(m_texFoxes[static_cast<int>(type)], row, spawnX, type));
    ++m_foxesSpawned;
}

void Game::SpawnSandboxGroup()
{
    if (!m_sandbox) return;
    const FoxType group[] = {
        FoxType::Common, FoxType::Sprint, FoxType::Armored,
        FoxType::Jumper, FoxType::Miner, FoxType::Common,
        FoxType::Sprint, FoxType::Common, FoxType::Armored,
        FoxType::Alpha
    };
    for (FoxType type : group) SpawnSandboxFox(type);
}

void Game::StartSandbox()
{
    if (!m_sandbox) m_savedCampaignEggs = m_eggs;
    m_sandbox = true;
    m_eggs = 999999;
    m_birds.clear();
    m_foxes.clear();
    m_projectiles.clear();
    m_skyEggs.clear();
    m_particles.clear();
    m_grid.Clear();
    m_foxesSpawned = 0;
    m_foxesToSpawn = 0;
    m_spawnTimer = 999999.0f;
    m_skyEggTimer = 999999.0f;
    m_levelMessageTimer = 0.0f;
    m_campaignWon = false;
    m_paused = false;
    m_sandboxKeyWasDown.fill(false);
    UpdateWindowTitle();
}

void Game::ResetSandboxField()
{
    if (!m_sandbox) return;
    m_birds.clear();
    m_foxes.clear();
    m_projectiles.clear();
    m_skyEggs.clear();
    m_particles.clear();
    m_grid.Clear();
    m_foxesSpawned = 0;
    m_eggs = 999999;
}

bool Game::FoxInFront(const Bird& bird) const
{
    for (const auto& fox : m_foxes)
        if (fox->active && fox->canBeHit && fox->row == bird.row &&
            fox->position.x > bird.position.x && fox->position.x < (float)WIDTH)
            return true;
    return false;
}

void Game::ShootFrom(const Bird& bird)
{
    glm::vec2 pos(bird.position.x + bird.size.x * 0.75f,
                  bird.position.y + bird.size.y * 0.5f - 14.0f);
    bool slows = bird.type == BirdType::MudDuck;
    bool stuns = bird.type == BirdType::Owl;
    bool knockback = bird.type == BirdType::WarGoose;
    bool piercing = bird.type == BirdType::Emu;
    m_projectiles.push_back(std::make_unique<Projectile>(m_texGrain, pos, bird.attackDamage, slows, stuns, knockback, piercing));
}

void Game::ResolveCollisions(float dt)
{
    for (auto& fox : m_foxes)
    {
        if (!fox->active) continue;
        fox->eating = false;
        for (auto& bird : m_birds)
        {
            if (!bird->active || bird->row != fox->row || !fox->canBeHit) continue;
            if (!bird->Overlaps(*fox)) continue;

            if (fox->type == FoxType::Jumper && !fox->hasJumped)
            {
                fox->position.x = bird->position.x - fox->size.x - 2.0f;
                fox->hasJumped = true;
                continue;
            }

            if (bird->type == BirdType::BomberPigeon)
            {
                fox->TakeDamage(bird->attackDamage);
                SpawnParticles(fox->position + fox->size * 0.5f, glm::vec4(1.0f, 0.55f, 0.18f, 1.0f), 22);
                PlayEffect("audio/explosion.wav");
                bird->active = false;
                continue;
            }
            if (bird->type == BirdType::WarriorRooster)
            {
                if (bird->ReadyToShoot())
                {
                    fox->TakeDamage(bird->attackDamage);
                    bird->ResetCooldown();
                }
                fox->eating = true;
                bird->TakeDamage(fox->damagePerSecond * dt * 0.5f);
                continue;
            }
            fox->eating = true;
            bird->TakeDamage(fox->damagePerSecond * dt);
        }
    }

    for (auto& proj : m_projectiles)
    {
        if (!proj->active) continue;
        for (auto& fox : m_foxes)
        {
            if (!fox->active || !fox->canBeHit) continue;
            if (proj->Overlaps(*fox))
            {
                float oldHealth = fox->health + fox->armorHealth;
                fox->TakeDamage(proj->damage);
                glm::vec4 hitColor = (fox->type == FoxType::Armored)
                    ? glm::vec4(0.72f, 0.82f, 0.95f, 1.0f)
                    : glm::vec4(1.0f, 0.55f, 0.22f, 1.0f);
                SpawnParticles(fox->position + fox->size * 0.5f, hitColor, fox->type == FoxType::Armored ? 9 : 5);
                if (oldHealth > 0.0f && !fox->active)
                {
                    SpawnParticles(fox->position + fox->size * 0.5f, glm::vec4(0.95f, 0.78f, 0.55f, 1.0f), 18);
                    PlayEffect("audio/hit.wav");
                }
                if (proj->slows) fox->ApplySlow(3.0f);
                if (proj->stuns) fox->ApplyStun(1.2f);
                if (proj->knockback) fox->position.x += 95.0f;
                if (proj->piercing) proj->position.x += 100.0f;
                else proj->active = false;
                break;
            }
        }
    }
}

void Game::Cleanup()
{
    for (auto& bird : m_birds)
    {
        if (!bird->active)
        {
            m_grid.SetOccupied(bird->row, bird->col, false);
            SpawnParticles(bird->position + bird->size * 0.5f, glm::vec4(0.95f, 0.92f, 0.78f, 1.0f), 12);
            PlayEffect("audio/hit.wav");
        }
    }
    RemoveInactive(m_birds);
    RemoveInactive(m_foxes);
    RemoveInactive(m_projectiles);
    RemoveInactive(m_skyEggs);
}

void Game::LoadProgress()
{
    std::ifstream file("save.json");
    if (!file.is_open()) return;
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string data = buffer.str();
    m_eggs = std::max(0, ReadJsonInt(data, "eggs", 150));
    m_unlockedLevel = std::clamp(ReadJsonInt(data, "unlockedLevel", 1), 1, 10);
    m_currentLevel = std::clamp(ReadJsonInt(data, "currentLevel", m_unlockedLevel), 1, m_unlockedLevel);
    m_totalCompleted = std::max(0, ReadJsonInt(data, "completedLevels", 0));
}

void Game::SaveProgress()
{
    std::ofstream file("save.json");
    if (!file.is_open())
    {
        std::cerr << "[Save] Nie mozna zapisac save.json\n";
        return;
    }
    file << "{\n"
         << "  \"eggs\": " << m_eggs << ",\n"
         << "  \"currentLevel\": " << m_currentLevel << ",\n"
         << "  \"unlockedLevel\": " << m_unlockedLevel << ",\n"
         << "  \"completedLevels\": " << m_totalCompleted << "\n"
         << "}\n";
}

void Game::StartLevel(int level)
{
    m_currentLevel = std::clamp(level, 1, 10);
    m_birds.clear();
    m_foxes.clear();
    m_projectiles.clear();
    m_skyEggs.clear();
    m_particles.clear();
    m_grid.Clear();
    m_foxesSpawned = 0;
    m_foxesToSpawn = 5 + m_currentLevel * 2;
    m_spawnTimer = FIRST_FOX_DELAY;
    m_skyEggTimer = 4.0f;
    m_levelMessageTimer = 0.0f;
    m_campaignWon = false;
    SaveProgress();
    UpdateWindowTitle();
}

void Game::CompleteLevel()
{
    ++m_totalCompleted;
    m_eggs += 50 + m_currentLevel * 10;
    PlayEffect("audio/level.wav");
    if (m_currentLevel < 10)
    {
        m_unlockedLevel = std::max(m_unlockedLevel, m_currentLevel + 1);
        ++m_currentLevel;
        m_levelMessageTimer = 3.0f;
    }
    else
    {
        m_campaignWon = true;
        m_levelMessageTimer = 0.0f;
    }
    SaveProgress();
    UpdateWindowTitle();
}

void Game::SpawnSkyEgg()
{
    std::uniform_real_distribution<float> xDist(Grid::ORIGIN_X + 10.0f, Grid::ORIGIN_X + Grid::COLS * Grid::CELL_W - 35.0f);
    auto egg = std::make_unique<GameObject>(m_texWhite, glm::vec2(xDist(m_rng), 112.0f), glm::vec2(26.0f));
    egg->velocity = glm::vec2(0.0f, 38.0f);
    egg->color = glm::vec4(1.0f, 0.82f, 0.15f, 1.0f);
    egg->hitboxScale = 1.0f;
    m_skyEggs.push_back(std::move(egg));
}

void Game::ResetMatch()
{
    std::cout << "Lisy dotarly do kurnika! Powtarzasz poziom " << m_currentLevel << ".\n";
    StartLevel(m_currentLevel);
}

void Game::UpdateWindowTitle()
{
    if (m_window)
        glfwSetWindowTitle(m_window, "Feathers & Fangs: Chickens vs Foxes");
}

void Game::PlayEffect(const char* file)
{
#ifdef _WIN32
    PlaySoundA(file, nullptr, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
#else
    (void)file;
#endif
}

void Game::SpawnParticles(const glm::vec2& position, const glm::vec4& color, int count)
{
    std::uniform_real_distribution<float> speed(-95.0f, 95.0f);
    std::uniform_real_distribution<float> life(0.25f, 0.75f);
    std::uniform_real_distribution<float> size(3.0f, 8.0f);
    for (int i = 0; i < count; ++i)
    {
        Particle particle;
        particle.position = position;
        particle.velocity = glm::vec2(speed(m_rng), speed(m_rng) - 25.0f);
        particle.life = life(m_rng);
        particle.maxLife = particle.life;
        particle.size = size(m_rng);
        particle.color = color;
        m_particles.push_back(particle);
    }
}

void Game::DrawGameText(const std::string& text, float x, float y, float scale, const glm::vec4& color)
{
    std::vector<glm::vec4> pixels;
    for (char c : text)
    {
        if (c == ' ')
        {
            x += 4.0f * scale;
            continue;
        }
        std::array<unsigned char, 5> glyph = Glyph(c);
        for (int row = 0; row < 5; ++row)
            for (int col = 0; col < 5; ++col)
                if (glyph[row] & (1 << (4 - col)))
                    pixels.push_back(glm::vec4(x + col * scale, y + row * scale, scale, scale));
        x += 6.0f * scale;
    }
    m_renderer->DrawBatch(*m_texWhite, pixels, color);
}

void Game::DrawHud()
{
    const int costs[9] = { 100, 50, 175, 125, 150, 200, 125, 225, 350 };
    const char* shortNames[9] = { "KURA", "NIOSKA", "KOGUT", "INDYK", "KACZKA", "SOWA", "GOLEB", "GES", "EMU" };
    const char* descriptions[9] = {
        "ATAK DYSTANSOWY - STRZELA ZIARNEM W LISA",
        "EKONOMIA - CO 8 SEKUND PRODUKUJE 25 JAJEK",
        "WALKA WRECZ - ZADAJE DUZE OBRAZENIA Z BLISKA",
        "TANK - MA DUZO ZDROWIA I BLOKUJE PRZECIWNIKOW",
        "BLOTNY ATAK - SPOWALNIA TRAFIONE LISY",
        "SOWA - STUNUJE TRAFIONE LISY NA CHWILE",
        "BOMBA - WYBUCHA PRZY KONTAKCIE Z LISEM",
        "GES - ODPYCHA LISY DO TYLNIEJ CZESCI PLANSZY",
        "EMU - MOCNY POCISK PRZEBIJAJACY PRZECIWNIKOW"
    };

    // Glowny pasek jednostek.
    m_renderer->Draw(*m_texWhite, glm::vec2(0.0f, 0.0f), glm::vec2(WIDTH, 168.0f), 0.0f,
                     glm::vec4(0.075f, 0.095f, 0.085f, 1.0f));
    m_renderer->Draw(*m_texWhite, glm::vec2(0.0f, 106.0f), glm::vec2(WIDTH, 2.0f), 0.0f,
                     glm::vec4(0.62f, 0.43f, 0.18f, 1.0f));

    // Licznik jajek.
    m_renderer->Draw(*m_texWhite, glm::vec2(12.0f, 12.0f), glm::vec2(86.0f, 84.0f), 0.0f,
                     glm::vec4(0.48f, 0.32f, 0.12f, 1.0f));
    m_renderer->Draw(*m_texWhite, glm::vec2(16.0f, 16.0f), glm::vec2(78.0f, 76.0f), 0.0f,
                     glm::vec4(0.18f, 0.14f, 0.09f, 1.0f));
    DrawGameText("JAJKA", 27.0f, 28.0f, 2.0f, glm::vec4(1.0f, 0.82f, 0.38f, 1.0f));
    DrawGameText(m_sandbox ? "INF" : std::to_string(m_eggs), 27.0f, 57.0f, 2.8f, glm::vec4(1.0f));

    for (int i = 0; i < 9; ++i)
    {
        const float x = 110.0f + i * 128.0f;
        const bool selected = (i == m_selectedBird);
        const glm::vec4 border = selected ? glm::vec4(0.88f, 0.66f, 0.27f, 1.0f)
                                          : glm::vec4(0.25f, 0.29f, 0.25f, 1.0f);
        const glm::vec4 card = selected ? glm::vec4(0.29f, 0.25f, 0.16f, 1.0f)
                                        : glm::vec4(0.13f, 0.17f, 0.14f, 1.0f);
        m_renderer->Draw(*m_texWhite, glm::vec2(x - 2.0f, 7.0f), glm::vec2(122.0f, 98.0f), 0.0f, border);
        m_renderer->Draw(*m_texWhite, glm::vec2(x, 9.0f), glm::vec2(118.0f, 94.0f), 0.0f, card);
        m_renderer->Draw(*m_texWhite, glm::vec2(x, 9.0f), glm::vec2(118.0f, 4.0f), 0.0f,
                         selected ? glm::vec4(0.95f, 0.70f, 0.25f, 1.0f)
                                  : glm::vec4(0.34f, 0.40f, 0.32f, 1.0f));
        Bird birdPreview(m_texBirds[i], 0, 0, BIRD_TYPES[i]);
        m_renderer->Draw(*m_texBirds[i], glm::vec2(x + 38.0f, 14.0f), glm::vec2(42.0f, 42.0f), 0.0f,
                         birdPreview.color);
        DrawGameText(std::to_string(i + 1) + " " + shortNames[i], x + 8.0f, 62.0f, 1.25f,
                     selected ? glm::vec4(1.0f, 0.88f, 0.60f, 1.0f) : glm::vec4(0.90f, 0.92f, 0.87f, 1.0f));
        DrawGameText(std::to_string(costs[i]) + " JAJ", x + 22.0f, 82.0f, 1.05f,
                     glm::vec4(0.95f, 0.79f, 0.43f, 1.0f));
    }

    // Pasek opisu wybranej jednostki - zawsze widoczny wewnatrz okna.
    m_renderer->Draw(*m_texWhite, glm::vec2(12.0f, 116.0f), glm::vec2(1256.0f, 42.0f), 0.0f,
                     glm::vec4(0.12f, 0.15f, 0.12f, 1.0f));
    m_renderer->Draw(*m_texWhite, glm::vec2(12.0f, 116.0f), glm::vec2(5.0f, 42.0f), 0.0f,
                     glm::vec4(0.88f, 0.65f, 0.25f, 1.0f));
    DrawGameText("WYBRANO: " + std::string(shortNames[m_selectedBird]), 28.0f, 130.0f, 1.55f,
                 glm::vec4(1.0f, 0.84f, 0.48f, 1.0f));
    DrawGameText(descriptions[m_selectedBird], 290.0f, 130.0f, 1.45f,
                 glm::vec4(0.92f, 0.94f, 0.88f, 1.0f));

    // Informacje o poziomie i fali sa w pasku HUD, poza obszarem planszy.
    if (m_sandbox)
    {
        DrawGameText("TRYB SANDBOX", 850.0f, 130.0f, 1.35f,
                     glm::vec4(0.65f, 0.95f, 0.72f, 1.0f));
        DrawGameText("LISY: " + std::to_string(m_foxes.size()), 1040.0f, 130.0f, 1.35f,
                     glm::vec4(1.0f, 0.92f, 0.72f, 1.0f));
        DrawGameText("SANDBOX - NIESKONCZONE JAJKA", 28.0f, 151.0f, 1.05f,
                     glm::vec4(0.65f, 0.95f, 0.72f, 1.0f));
        DrawGameText("F1-F6 LISY  F7 FALA  F8 WYCZYSC  F9 RESET", 650.0f, 151.0f, 1.05f,
                     glm::vec4(0.82f, 0.88f, 0.80f, 1.0f));
    }
    else
    {
        DrawGameText("POZ " + std::to_string(m_currentLevel) + "/10", 850.0f, 130.0f, 1.35f,
                     glm::vec4(1.0f, 0.92f, 0.72f, 1.0f));
        DrawGameText("LISY " + std::to_string(m_foxesSpawned) + "/" + std::to_string(m_foxesToSpawn), 1040.0f, 130.0f, 1.35f,
                     glm::vec4(1.0f, 0.92f, 0.72f, 1.0f));
    }
}

void Game::DrawMenu()
{
    m_renderer->Draw(*m_texWhite, glm::vec2(0.0f), glm::vec2(WIDTH, HEIGHT), 0.0f,
                     glm::vec4(0.035f, 0.060f, 0.050f, 1.0f));

    // Delikatne pasy tla i centralny panel.
    for (int i = 0; i < 8; ++i)
        m_renderer->Draw(*m_texWhite, glm::vec2(0.0f, 80.0f + i * 82.0f), glm::vec2(WIDTH, 1.0f), 0.0f,
                         glm::vec4(0.13f, 0.17f, 0.12f, 0.45f));

    m_renderer->Draw(*m_texWhite, glm::vec2(72.0f, 48.0f), glm::vec2(1136.0f, 624.0f), 0.0f,
                     glm::vec4(0.34f, 0.27f, 0.16f, 1.0f));
    m_renderer->Draw(*m_texWhite, glm::vec2(78.0f, 54.0f), glm::vec2(1124.0f, 612.0f), 0.0f,
                     glm::vec4(0.075f, 0.105f, 0.083f, 1.0f));

    m_renderer->Draw(*m_texWhite, glm::vec2(130.0f, 142.0f), glm::vec2(1020.0f, 2.0f), 0.0f,
                     glm::vec4(0.60f, 0.43f, 0.21f, 1.0f));
    DrawGameText("FEATHERS & FANGS", 300.0f, 82.0f, 5.5f,
                 glm::vec4(0.96f, 0.79f, 0.43f, 1.0f));
    DrawGameText("CHICKENS VS FOXES", 430.0f, 120.0f, 1.8f,
                 glm::vec4(0.82f, 0.88f, 0.80f, 1.0f));

    if (m_showHelp)
    {
        DrawHelpMenu();
        return;
    }

    DrawGameText("WYBIERZ TRYB GRY", 185.0f, 190.0f, 2.0f,
                 glm::vec4(0.92f, 0.89f, 0.76f, 1.0f));

    const char* options[4] = { "KAMPANIA", "SANDBOX", "JAK GRAC", "WYJSCIE" };
    const char* subtitles[4] = {
        "10 POZIOMOW I ZAPIS POSTEPU",
        "TESTUJ JEDNOSTKI I SPAWNUJ LISY BEZ LIMITU",
        "STEROWANIE I OPIS TRYBOW",
        "ZAMKNIJ FEATHERS & FANGS"
    };

    for (int i = 0; i < 4; ++i)
    {
        float y = 245.0f + i * 88.0f;
        bool selected = (i == m_menuSelection);
        glm::vec4 edge = selected ? glm::vec4(0.91f, 0.67f, 0.27f, 1.0f)
                                  : glm::vec4(0.24f, 0.31f, 0.25f, 1.0f);
        glm::vec4 fill = selected ? glm::vec4(0.28f, 0.25f, 0.15f, 1.0f)
                                  : glm::vec4(0.11f, 0.15f, 0.12f, 1.0f);
        m_renderer->Draw(*m_texWhite, glm::vec2(145.0f, y), glm::vec2(470.0f, 68.0f), 0.0f, edge);
        m_renderer->Draw(*m_texWhite, glm::vec2(149.0f, y + 4.0f), glm::vec2(462.0f, 60.0f), 0.0f, fill);
        DrawGameText(std::to_string(i + 1) + "  " + options[i], 175.0f, y + 13.0f, 2.3f,
                     selected ? glm::vec4(1.0f, 0.83f, 0.43f, 1.0f)
                              : glm::vec4(0.90f, 0.93f, 0.87f, 1.0f));
        DrawGameText(subtitles[i], 175.0f, y + 43.0f, 0.85f,
                     glm::vec4(0.69f, 0.76f, 0.68f, 1.0f));
    }

    // Panel informacyjny po prawej stronie menu.
    m_renderer->Draw(*m_texWhite, glm::vec2(680.0f, 190.0f), glm::vec2(430.0f, 390.0f), 0.0f,
                     glm::vec4(0.12f, 0.16f, 0.12f, 1.0f));
    m_renderer->Draw(*m_texWhite, glm::vec2(680.0f, 190.0f), glm::vec2(5.0f, 390.0f), 0.0f,
                     glm::vec4(0.68f, 0.49f, 0.23f, 1.0f));
    DrawGameText("INFORMACJE", 720.0f, 220.0f, 2.0f,
                 glm::vec4(0.96f, 0.79f, 0.43f, 1.0f));

    if (m_menuSelection == 0)
    {
        DrawGameText("KAMPANIA", 720.0f, 270.0f, 2.2f, glm::vec4(1.0f));
        DrawGameText("BRON GOSPODARSTWA", 720.0f, 320.0f, 1.35f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("PRZECHODZ KOLEJNE FALE LISOW", 720.0f, 350.0f, 1.15f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("POSTEP ZAPISUJE SIE AUTOMATYCZNIE", 720.0f, 380.0f, 1.0f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("ODBLOKOWANO: " + std::to_string(m_unlockedLevel) + "/10", 720.0f, 445.0f, 1.5f,
                     glm::vec4(0.96f, 0.79f, 0.43f, 1.0f));
    }
    else if (m_menuSelection == 1)
    {
        DrawGameText("TRYB TESTOWY", 720.0f, 270.0f, 2.2f, glm::vec4(1.0f));
        DrawGameText("NIESKONCZONE JAJKA", 720.0f, 320.0f, 1.4f, glm::vec4(0.65f, 0.95f, 0.72f, 1.0f));
        DrawGameText("SPAWNUJ 6 TYPOW LISOW", 720.0f, 355.0f, 1.2f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("BEZ AUTOMATYCZNYCH FAL", 720.0f, 390.0f, 1.2f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("BEZ PRZEGRANEJ", 720.0f, 425.0f, 1.2f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
    }
    else if (m_menuSelection == 2)
    {
        DrawGameText("PODSTAWOWE STEROWANIE", 720.0f, 270.0f, 1.6f, glm::vec4(1.0f));
        DrawGameText("1-9 WYBOR PTAKA", 720.0f, 320.0f, 1.3f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("LPM STAWIA I ZBIERA JAJKA", 720.0f, 355.0f, 1.1f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("P PAUZA - ESC MENU", 720.0f, 390.0f, 1.3f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
        DrawGameText("ENTER OTWIERA WYBRANY TRYB", 720.0f, 445.0f, 1.0f, glm::vec4(0.96f, 0.79f, 0.43f, 1.0f));
    }
    else
    {
        DrawGameText("DO ZOBACZENIA!", 720.0f, 285.0f, 2.0f, glm::vec4(1.0f));
        DrawGameText("ZAMKNIECIE GRY", 720.0f, 335.0f, 1.5f, glm::vec4(0.85f, 0.90f, 0.82f, 1.0f));
    }

    DrawGameText("W/S LUB STRZALKI - WYBOR     ENTER - ZATWIERDZ     ESC - WYJSCIE", 205.0f, 625.0f, 1.25f,
                 glm::vec4(0.70f, 0.77f, 0.69f, 1.0f));
}

void Game::DrawHelpMenu()
{
    m_renderer->Draw(*m_texWhite, glm::vec2(145.0f, 180.0f), glm::vec2(990.0f, 400.0f), 0.0f,
                     glm::vec4(0.12f, 0.16f, 0.12f, 1.0f));
    DrawGameText("JAK GRAC", 500.0f, 210.0f, 3.0f, glm::vec4(0.96f, 0.79f, 0.43f, 1.0f));
    DrawGameText("KAMPANIA: PRZETRWAJ WSZYSTKIE FALE I ODBLOKUJ 10 POZIOMOW", 205.0f, 285.0f, 1.25f, glm::vec4(1.0f));
    DrawGameText("SANDBOX: MASZ NIESKONCZONE JAJKA I SAM SPAWNUJESZ LISY", 205.0f, 325.0f, 1.25f, glm::vec4(1.0f));
    DrawGameText("1-9 WYBIERA PTAKA, LPM STAWIA GO NA WOLNYM POLU", 205.0f, 385.0f, 1.25f, glm::vec4(0.86f, 0.90f, 0.82f, 1.0f));
    DrawGameText("F1 ZWYKLY  F2 SPRINT  F3 PANCERNY  F4 SKOCZEK", 205.0f, 425.0f, 1.25f, glm::vec4(0.86f, 0.90f, 0.82f, 1.0f));
    DrawGameText("F5 GORNIK  F6 ALFA  F7 GRUPA  F8 USUN LISY  F9 RESET", 205.0f, 465.0f, 1.25f, glm::vec4(0.86f, 0.90f, 0.82f, 1.0f));
    DrawGameText("P PAUZA, ESC POWROT DO MENU, ENTER POWROT", 205.0f, 525.0f, 1.25f, glm::vec4(0.96f, 0.79f, 0.43f, 1.0f));
    DrawGameText("NACISNIJ ENTER LUB ESC ABY WROCIC", 410.0f, 585.0f, 1.5f, glm::vec4(1.0f));
}

void Game::Update(float dt)
{
    if (m_inMenu || m_paused) return;
    for (auto& particle : m_particles)
    {
        particle.life -= dt;
        particle.position += particle.velocity * dt;
        particle.velocity.y += 95.0f * dt;
        particle.color.a = std::max(0.0f, particle.life / particle.maxLife);
    }
    m_particles.erase(std::remove_if(m_particles.begin(), m_particles.end(),
        [](const Particle& particle) { return particle.life <= 0.0f; }), m_particles.end());
    if (m_campaignWon) return;
    if (m_levelMessageTimer > 0.0f)
    {
        m_levelMessageTimer -= dt;
        if (m_levelMessageTimer <= 0.0f) StartLevel(m_currentLevel);
        return;
    }

    HandlePlacement();

    if (!m_sandbox)
    {
        m_skyEggTimer -= dt;
        if (m_skyEggTimer <= 0.0f)
        {
            SpawnSkyEgg();
            m_skyEggTimer = SKY_EGG_INTERVAL;
        }

        m_spawnTimer -= dt;
        if (m_spawnTimer <= 0.0f && m_foxesSpawned < m_foxesToSpawn)
        {
            SpawnFox();
            float interval = std::max(1.8f, 6.0f - m_currentLevel * 0.35f);
            m_spawnTimer = interval;
        }
    }

    for (auto& bird : m_birds)
    {
        bird->Update(dt);
        if (!m_sandbox && bird->type == BirdType::LayingHen && bird->eggTimer <= 0.0f)
        {
            m_eggs += 25;
            bird->eggTimer = 8.0f;
            SaveProgress();
        }
        if (bird->IsRanged() && bird->ReadyToShoot() && FoxInFront(*bird))
        {
            ShootFrom(*bird);
            bird->ResetCooldown();
        }
    }

    bool spawnMinions = false;
    int alphaRow = 0;
    float alphaX = 0.0f;
    for (auto& fox : m_foxes)
    {
        if (fox->active && fox->type == FoxType::Alpha && !fox->spawnedMinions && fox->health <= fox->maxHealth * 0.5f)
        {
            fox->spawnedMinions = true;
            spawnMinions = true;
            alphaRow = fox->row;
            alphaX = fox->position.x;
        }
        fox->Update(dt);
        if (m_sandbox && fox->position.x + fox->size.x < 0.0f) fox->active = false;
    }
    if (spawnMinions)
    {
        m_foxes.push_back(std::make_unique<Fox>(m_texFoxes[static_cast<int>(FoxType::Common)], alphaRow, alphaX + 35.0f, FoxType::Common));
        m_foxes.push_back(std::make_unique<Fox>(m_texFoxes[static_cast<int>(FoxType::Sprint)], std::min(Grid::ROWS - 1, alphaRow + 1), alphaX + 65.0f, FoxType::Sprint));
    }

    for (auto& proj : m_projectiles)
    {
        proj->Update(dt);
        if (proj->position.x > (float)WIDTH) proj->active = false;
    }
    for (auto& egg : m_skyEggs)
    {
        egg->Update(dt);
        if (egg->position.y > HEIGHT) egg->active = false;
    }

    ResolveCollisions(dt);
    Cleanup();

    if (!m_sandbox)
    {
        for (const auto& fox : m_foxes)
        {
            if (fox->position.x + fox->size.x < Grid::ORIGIN_X)
            {
                ResetMatch();
                return;
            }
        }

        if (m_foxesSpawned >= m_foxesToSpawn && m_foxes.empty()) CompleteLevel();
    }
    else
    {
        m_eggs = 999999;
    }
    UpdateWindowTitle();
}

void Game::Render()
{
    if (!m_renderer) return;
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_spriteShader.Use();
    m_spriteShader.SetInt("nightMode", (!m_inMenu && m_currentLevel >= 7) ? 1 : 0);
    m_spriteShader.SetVec2("screenSize", glm::vec2((float)WIDTH, (float)HEIGHT));

    if (m_inMenu)
    {
        DrawMenu();
        glfwSwapBuffers(m_window);
        return;
    }

    glm::vec4 background(0.28f, 0.50f, 0.22f, 1.0f);
    if (m_currentLevel >= 4 && m_currentLevel <= 6) background = glm::vec4(0.40f, 0.29f, 0.19f, 1.0f);
    if (m_currentLevel >= 7) background = glm::vec4(0.12f, 0.20f, 0.24f, 1.0f);

    m_renderer->Draw(*m_texWhite, glm::vec2(0.0f), glm::vec2(WIDTH, HEIGHT), 0.0f, background);
    m_grid.Draw(*m_renderer, *m_texWhite, m_hoverRow, m_hoverCol);
    for (const auto& bird : m_birds) bird->Draw(*m_renderer);
    for (const auto& fox : m_foxes)
    {
        fox->Draw(*m_renderer);
        float ratio = std::clamp(fox->health / fox->maxHealth, 0.0f, 1.0f);
        glm::vec2 barPos(fox->position.x, fox->position.y - 8.0f);
        m_renderer->Draw(*m_texWhite, barPos, glm::vec2(fox->size.x, 5.0f), 0.0f, glm::vec4(0.25f, 0.05f, 0.05f, 1.0f));
        m_renderer->Draw(*m_texWhite, barPos, glm::vec2(fox->size.x * ratio, 5.0f), 0.0f, glm::vec4(0.2f, 0.9f, 0.2f, 1.0f));
    }
    for (const auto& proj : m_projectiles) proj->Draw(*m_renderer);
    for (const auto& egg : m_skyEggs) egg->Draw(*m_renderer);
    for (const auto& particle : m_particles)
        m_renderer->Draw(*m_texWhite, particle.position, glm::vec2(particle.size), 0.0f, particle.color);

    DrawHud();
    if (m_paused)
    {
        m_renderer->Draw(*m_texWhite, glm::vec2(0.0f), glm::vec2(WIDTH, HEIGHT), 0.0f,
                         glm::vec4(0.0f, 0.0f, 0.0f, 0.58f));
        DrawGameText("PAUZA", 520.0f, 280.0f, 8.0f, glm::vec4(1.0f, 0.85f, 0.35f, 1.0f));
        DrawGameText("PRESS P TO CONTINUE", 420.0f, 370.0f, 3.0f, glm::vec4(1.0f));
    }
    if (m_campaignWon)
    {
        m_renderer->Draw(*m_texWhite, glm::vec2(0.0f), glm::vec2(WIDTH, HEIGHT), 0.0f,
                         glm::vec4(0.0f, 0.0f, 0.0f, 0.55f));
        DrawGameText("YOU WIN", 490.0f, 280.0f, 8.0f, glm::vec4(1.0f, 0.85f, 0.35f, 1.0f));
    }
    if (m_levelMessageTimer > 0.0f)
    {
        m_renderer->Draw(*m_texWhite, glm::vec2(300.0f, 290.0f), glm::vec2(680.0f, 100.0f), 0.0f,
                         glm::vec4(0.12f, 0.10f, 0.08f, 0.88f));
        DrawGameText("POZIOM UKONCZONY", 400.0f, 325.0f, 4.0f, glm::vec4(1.0f, 0.85f, 0.35f, 1.0f));
    }
    glfwSwapBuffers(m_window);
}

void Game::Shutdown()
{
    m_birds.clear();
    m_foxes.clear();
    m_projectiles.clear();
    m_skyEggs.clear();
    m_particles.clear();
    for (auto& texture : m_texBirds) texture.reset();
    for (auto& texture : m_texFoxes) texture.reset();
    m_texGrain.reset();
    m_texWhite.reset();
    m_renderer.reset();
    m_spriteShader.Release();
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
    }
}
