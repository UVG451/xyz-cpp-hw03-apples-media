#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <unordered_map>
#include <string>

#include "Player.h"
#include "Apple.h"
#include "Obstacle.h"
#include "UI.h"
#include "GameSettings.h"

namespace ApplesGame
{
    struct Record
    {
        std::string name;
        int score = 0;
    };

    struct GameState
    {
        Player player;

        // динамический массив €блок
        Apple* apples = nullptr;
        int applesCount = 0;          // текущее количество на поле
        int applesInitialCount = 20;

        // преп€тстви€
        Obstacle obstacles[MAX_OBSTACLES];
        int currentObstaclesCount = BASE_NUM_OBSTACLES;

        int numEatenApples = 0;
        int bestScore = 0;

        UIState uiState;

        bool isGameOver = false;
        bool restartAfterWall = false;
        float timeSinceGameOver = 0.f;

        // –есурсы
        sf::Font font;
        sf::Texture playerTexture;
        sf::Texture appleTexture;
        sf::Texture obstacleTexture;

        // «вуки
        sf::SoundBuffer eatAppleBuffer;
        sf::SoundBuffer hitBuffer;
        sf::Sound eatAppleSound;
        sf::Sound hitSound;

        // –ежим игры (битова€ маска)
        unsigned int gameModeMask = 0;

        // скорость
        float playerBaseSpeed = INITIAL_SPEED;
        float playerAcceleration = ACCELERATION;

        // таблица лидеров
        std::vector<Record> leaderboard;
        std::unordered_map<std::string, int> leaderboardScores;
    };

    void InitGame(GameState& gameState);
    void RestartGame(GameState& gameState);
    void HandleInput(GameState& gameState);
    void UpdateGame(GameState& gameState, float timeDelta);
    void DrawGame(GameState& gameState, sf::RenderWindow& window);
}
