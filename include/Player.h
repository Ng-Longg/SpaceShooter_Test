#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "PowerUp.h"

struct ActivePower {
    PowerType type;
    sf::Clock timer;
    float duration;
};

class Player {
private:
    sf::RectangleShape shape;
    sf::Texture texture;
    sf::Sprite sprite;

    int hp;
    sf::Clock invincibleClock;
    float invincibleDuration;

    std::vector<ActivePower> activePowers;
    float playerSize;

public:
    Player();

    void update(sf::RenderWindow& window);
    void render(sf::RenderWindow& window);

    sf::Vector2f getPos() const;
    sf::FloatRect getBounds() const;
    int getHP() const { return hp; }

    void takeDamage(int dmg);
    bool isInvincible() const;
    void resetHP();

    void addWeapon(PowerType type, float duration);
    void applyPowerUp(PowerType type, float duration); // HP hồi ở đây

    const std::vector<ActivePower>& getActivePowers() const { return activePowers; }
};
