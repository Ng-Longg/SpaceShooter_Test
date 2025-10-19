#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Player.h"
#include "Bullet.h"
#include "Enemy.h"
#include "PowerUp.h"

class Game {
public:
    Game();
    void run();

private:
    sf::RenderWindow window;

    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<PowerUp> powerUps;
    std::vector<sf::RectangleShape> enemyBullets;

    sf::Clock shootTimer;
    sf::Clock enemySpawnTimer;
    sf::Clock difficultyClock;

    sf::Font font;
    sf::Text scoreText;
    sf::Text highScoreText;
    sf::Text hpText;
    sf::Text gameOverText;

    int score;
    int highScore;
    bool isGameOver;
    bool bossSpawned;

    int difficultyLevel;
    float enemySpawnInterval;
    float enemySpeedMultiplier;

    sf::Vector2f screenShakeOffset;
    float screenShakeMagnitude;
    float screenShakeDuration;
    sf::Clock screenShakeClock;

    std::vector<sf::CircleShape> explosions;
    std::vector<sf::Clock> explosionTimers;

    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

private:
    void processEvents();
    void update();
    void render();
    void spawnEnemy();
    void resetGame();
    void saveHighScore();
    void increaseDifficulty();
    void startScreenShake(float duration, float magnitude);
    void updateScreenShake();
    void createExplosion(sf::Vector2f position, sf::Color color = sf::Color::Yellow, float size = 10.f);
    void updateExplosions();
};
