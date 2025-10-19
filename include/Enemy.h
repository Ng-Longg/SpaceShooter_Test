#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <cstdlib>
#include <cmath>

enum class EnemyType { Normal, Fast, Tank, Speed, Boss };

class Enemy {
private:
    static sf::Texture enemyTexture;
    static sf::Texture bossTexture;
    static sf::Texture tankTexture;
    static sf::Texture speedTexture;

    sf::Sprite sprite;
    EnemyType type;
    float baseSpeed;
    int hp;
    int maxHp;
    bool canShoot;
    sf::Clock shootClock;
    float shootInterval;
    float moveDirection;

public:
    Enemy(sf::Vector2f startPos, EnemyType type = EnemyType::Normal, sf::Vector2f size = sf::Vector2f(40.f,40.f));

    static bool loadTextures();

    void update(sf::Vector2f playerPos, std::vector<sf::RectangleShape>& enemyBullets, float speedMultiplier = 1.f);
    void render(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    bool isDead() const;
    void takeDamage(int dmg);
    EnemyType getType() const;
    int getHp() const;
    int getMaxHp() const;

    void setSize(sf::Vector2f newSize);

    static const sf::Texture& getEnemyTexture() { return enemyTexture; }
    static const sf::Texture& getBossTexture() { return bossTexture; }
    static const sf::Texture& getTankTexture() { return tankTexture; }
    static const sf::Texture& getSpeedTexture() { return speedTexture; }
};
