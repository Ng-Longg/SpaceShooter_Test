#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
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

    // ===== AUDIO =====
    sf::Music bgMusic;                    // nhạc nền (stream)
    sf::SoundBuffer explosionBuffer;      // buffer cho âm thanh nổ
    sf::Sound explosionSound;             // sound dùng để phát nổ

    // ===== TIMERS =====
    sf::Clock shootTimer;
    sf::Clock enemySpawnTimer;
    sf::Clock difficultyClock;

    // ===== TEXT / UI =====
    sf::Font font;
    sf::Text scoreText;
    sf::Text highScoreText;
    sf::Text hpText;
    sf::Text gameOverText;
    sf::Text pauseText;

    // ===== GAME STATE =====
    int score;
    int highScore;
    bool isGameOver;
    bool bossSpawned;
    bool isPaused;        // 👈 Thêm biến này

    // ===== DIFFICULTY =====
    int difficultyLevel;
    float enemySpawnInterval;
    float enemySpeedMultiplier;

    // ===== SCREEN SHAKE =====
    sf::Vector2f screenShakeOffset;
    float screenShakeMagnitude;
    float screenShakeDuration;
    sf::Clock screenShakeClock;

    // ===== EFFECTS =====
    std::vector<sf::CircleShape> explosions;
    std::vector<sf::Clock> explosionTimers;

    // ===== BACKGROUND =====
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
