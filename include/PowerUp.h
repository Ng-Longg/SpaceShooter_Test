#pragma once
#include <SFML/Graphics.hpp>

enum class PowerType {
    HP,          // Xanh lá
    DoubleShot,  // Vàng
    HomingShot,  // Tím
    LaserBeam    // Đỏ
};

class PowerUp {
private:
    sf::CircleShape shape;
    PowerType type;
    float speed;

public:
    PowerUp(sf::Vector2f startPos, PowerType type);
    void update();
    void render(sf::RenderWindow& window);
    sf::FloatRect getBounds() const;
    PowerType getType() const;
    bool outOfWindow() const;
};
