#include "PowerUp.h"

PowerUp::PowerUp(sf::Vector2f startPos, PowerType type){
    this->type = type;
    shape.setRadius(15.f);
    shape.setOrigin(15.f,15.f);

    switch(type){
        case PowerType::HP: shape.setFillColor(sf::Color::Green); break;
        case PowerType::DoubleShot: shape.setFillColor(sf::Color::Yellow); break;
        case PowerType::HomingShot: shape.setFillColor(sf::Color(128,0,128)); break; // tím
        case PowerType::LaserBeam: shape.setFillColor(sf::Color::Red); break;
    }

    shape.setPosition(startPos);
    speed = 0.2f;
}

void PowerUp::update(){ shape.move(0.f,speed); }
void PowerUp::render(sf::RenderWindow& window){ window.draw(shape); }
sf::FloatRect PowerUp::getBounds() const { return shape.getGlobalBounds(); }
PowerType PowerUp::getType() const { return type; }
bool PowerUp::outOfWindow() const { return shape.getPosition().y>650.f; }
