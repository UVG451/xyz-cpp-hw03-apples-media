#include "Game.h"
#include "Math.h"

#include <iterator>
#include <random>
#include <stdexcept>
#include <algorithm>
#include <cstdlib>
#include <ctime>

namespace ApplesGame
{
    // ---------- вспомогательные данные для таблицы лидеров ----------

    const char* NAMES_POOL[] = {
        "Alice", "Bob", "Carol", "Dave", "Eve",
        "Frank", "Grace", "Heidi", "Ivan", "Judy"
    };

    // простая сортировка выбором по убыванию score (без std::sort)
    void SortLeaderboard(std::vector<Record>& board)
    {
        for (size_t i = 0; i + 1 < board.size(); ++i)
        {
            size_t bestIndex = i;
            for (size_t j = i + 1; j < board.size(); ++j)
            {
                if (board[j].score > board[bestIndex].score)
                    bestIndex = j;
            }
            if (bestIndex != i)
                std::swap(board[i], board[bestIndex]);
        }
    }

    void GenerateRandomLeaderboard(GameState& gameState, int count)
    {
        std::vector<std::string> names(std::begin(NAMES_POOL), std::end(NAMES_POOL));
        std::mt19937 random(std::random_device{}());
        std::shuffle(names.begin(), names.end(), random);
        std::uniform_int_distribution<int> score(20, 139);
        count = std::max(4, std::min(count, 9));
        for (int i = 0; i < count; ++i)
        {
            gameState.leaderboardScores.emplace(names[i], score(random));
        }
    }

    void UpdateLeaderboardWithPlayer(GameState& gameState)
    {
        gameState.leaderboardScores["Player"] = gameState.bestScore;
        gameState.leaderboard.clear();
        for (const auto& entry : gameState.leaderboardScores)
        {
            gameState.leaderboard.push_back({entry.first, entry.second});
        }
        SortLeaderboard(gameState.leaderboard);
    }

    // ---------- основная логика ----------

    void InitGame(GameState& gameState)
    {
        std::srand(static_cast<unsigned int>(std::time(nullptr)));

        // ресурсы
        if (!gameState.playerTexture.loadFromFile(RESOURCES_PATH + "Pacman.png"))
            throw std::runtime_error("Cannot load Pacman.png");
        if (!gameState.appleTexture.loadFromFile(RESOURCES_PATH + "Apple.png"))
            throw std::runtime_error("Cannot load Apple.png");
        if (!gameState.obstacleTexture.loadFromFile(RESOURCES_PATH + "Obstacle.png"))
            throw std::runtime_error("Cannot load Obstacle.png");
        if (!gameState.font.loadFromFile(RESOURCES_PATH + "Fonts/Roboto-Regular.ttf"))
            throw std::runtime_error("Cannot load Fonts/Roboto-Regular.ttf");

        // звуки
        if (!gameState.eatAppleBuffer.loadFromFile(RESOURCES_PATH + "Sounds/eat.wav"))
            throw std::runtime_error("Cannot load Sounds/eat.wav");
        if (!gameState.hitBuffer.loadFromFile(RESOURCES_PATH + "Sounds/hit.wav"))
            throw std::runtime_error("Cannot load Sounds/hit.wav");

        gameState.eatAppleSound.setBuffer(gameState.eatAppleBuffer);
        gameState.hitSound.setBuffer(gameState.hitBuffer);

        InitUI(gameState.uiState, gameState.font);

        // выделяем память под максимальное количество яблок один раз
        if (!gameState.apples)
        {
            gameState.apples = new Apple[MAX_APPLES];
        }

        RestartGame(gameState);
    }

    void RestartGame(GameState& gameState)
    {
        // первая инициализация leaderboard – делаем новую таблицу
        if (gameState.leaderboardScores.empty())
        {
            GenerateRandomLeaderboard(gameState, 7); // например, 7 записей
        }

        gameState.applesCount = std::max(1, std::min(gameState.applesInitialCount, MAX_APPLES));

        gameState.currentObstaclesCount = BASE_NUM_OBSTACLES;

        if (gameState.gameModeMask & MODE_SPEED_ACCEL_ON)
            gameState.playerAcceleration = ACCELERATION;
        else if (gameState.gameModeMask & MODE_SPEED_ACCEL_OFF)
            gameState.playerAcceleration = 0.f;
        else
            gameState.playerAcceleration = ACCELERATION;

        InitPlayer(gameState.player, gameState.playerTexture);
        gameState.player.speed = INITIAL_SPEED;

        // яблоки
        for (int i = 0; i < gameState.applesCount; ++i)
        {
            InitApple(gameState.apples[i], gameState.appleTexture);
        }

        // препятствия
        for (int i = 0; i < gameState.currentObstaclesCount; ++i)
        {
            InitObstacle(gameState.obstacles[i], gameState.obstacleTexture);
        }

        gameState.numEatenApples = 0;
        gameState.isGameOver = false;
        gameState.restartAfterWall = false;
        gameState.timeSinceGameOver = 0.f;
    }

    void HandleInput(GameState& gameState)
    {
        if (gameState.isGameOver)
        {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
            {
                RestartGame(gameState);
            }
            return;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            gameState.player.direction = PlayerDirection::Up;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            gameState.player.direction = PlayerDirection::Right;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            gameState.player.direction = PlayerDirection::Down;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            gameState.player.direction = PlayerDirection::Left;
        }
    }

    void UpdateGame(GameState& gameState, float timeDelta)
    {
        if (!gameState.isGameOver)
        {
            UpdatePlayer(gameState.player, timeDelta);

            // столкновения с яблоками
            for (int i = 0; i < gameState.applesCount; ++i)
            {
                if (HasPlayerCollisionWithApple(gameState.player, gameState.apples[i]))
                {
                    if (gameState.gameModeMask & MODE_APPLES_FINITE)
                    {
                        gameState.apples[i] = gameState.apples[gameState.applesCount - 1];
                        --gameState.applesCount;
                        --i;
                    }
                    else
                    {
                        InitApple(gameState.apples[i], gameState.appleTexture);
                    }

                    gameState.numEatenApples++;

                    if (gameState.numEatenApples > gameState.bestScore)
                        gameState.bestScore = gameState.numEatenApples;

                    gameState.eatAppleSound.play();

                    if (gameState.gameModeMask & MODE_SPEED_ACCEL_ON)
                    {
                        gameState.player.speed += gameState.playerAcceleration;
                    }

                    if ((gameState.gameModeMask & MODE_APPLES_FINITE) &&
                        gameState.applesCount == 0)
                    {
                        gameState.isGameOver = true;
                        gameState.timeSinceGameOver = 0.f;
                        UpdateLeaderboardWithPlayer(gameState);
                        break;
                    }
                }
            }

            // столкновения с препятствиями
            for (int i = 0; i < gameState.currentObstaclesCount && !gameState.isGameOver; ++i)
            {
                if (HasPlayerCollisionWithObstacle(gameState.player, gameState.obstacles[i]))
                {
                    gameState.isGameOver = true;
                    gameState.timeSinceGameOver = 0.f;
                    gameState.hitSound.play();
                    UpdateLeaderboardWithPlayer(gameState);
                }
            }

            // столкновение со стенкой
            if (!gameState.isGameOver && HasPlayerCollisionWithScreenBorder(gameState.player))
            {
                gameState.isGameOver = true;
                gameState.restartAfterWall = true;
                gameState.timeSinceGameOver = 0.f;
                gameState.hitSound.play();
                UpdateLeaderboardWithPlayer(gameState);
            }
        }
        else
        {
            gameState.timeSinceGameOver += timeDelta;
            if (gameState.restartAfterWall && gameState.timeSinceGameOver >= 3.f)
                RestartGame(gameState);
        }

        UpdateUI(gameState.uiState, gameState, timeDelta);
    }

    void DrawGame(GameState& gameState, sf::RenderWindow& window)
    {
        DrawPlayer(gameState.player, window);

        for (int i = 0; i < gameState.applesCount; ++i)
        {
            DrawApple(gameState.apples[i], window);
        }

        for (int i = 0; i < gameState.currentObstaclesCount; ++i)
        {
            DrawObstacle(gameState.obstacles[i], window);
        }

        DrawUI(gameState.uiState, window);
    }
}
