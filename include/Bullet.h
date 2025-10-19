#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include "Enemy.h"
#include "PowerUp.h"

class Bullet {
private:
    sf::RectangleShape shape;
    sf::Vector2f velocity;
    float speed;
    PowerType bulletType;

    sf::Clock lifeClock;       
    sf::Clock damageClock;     
    float maxLifeTime;         

public:
    Bullet(sf::Vector2f startPos, PowerType type);

    void update(std::vector<Enemy>& enemies);
    void followPlayer(const sf::Vector2f& playerCenter);
    void render(sf::RenderWindow& window);

    sf::FloatRect getBounds() const;
    bool outOfWindow() const;
    PowerType getType() const;

    friend class Game;
};
