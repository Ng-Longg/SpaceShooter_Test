#include "Player.h"
#include <iostream>
#include <algorithm>

Player::Player()
{
    playerSize = 100.f;
    shape.setSize(sf::Vector2f(playerSize, playerSize));
    shape.setFillColor(sf::Color::Cyan);
    shape.setPosition(375.f, 500.f);

    if (!texture.loadFromFile("assets/Player.png"))
        std::cerr << "Warning: could not load Player texture\n";

    sprite.setTexture(texture);
    sprite.setScale(playerSize / texture.getSize().x, playerSize / texture.getSize().y);
    sprite.setPosition(shape.getPosition());

    hp = 10;
    invincibleDuration = 0.5f;
    invincibleClock.restart();
}

void Player::update(sf::RenderWindow& window)
{
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    float newX = std::clamp(float(mousePos.x) - shape.getSize().x/2.f, 0.f, float(window.getSize().x - shape.getSize().x));
    float newY = std::clamp(float(mousePos.y) - shape.getSize().y/2.f, 0.f, float(window.getSize().y - shape.getSize().y));
    shape.setPosition(newX, newY);
    sprite.setPosition(shape.getPosition());

    // Remove expired powers
    for(int i=int(activePowers.size())-1;i>=0;--i)
        if(activePowers[i].timer.getElapsedTime().asSeconds() > activePowers[i].duration)
            activePowers.erase(activePowers.begin()+i);

    // Update color based on last power
    if(!activePowers.empty()){
        switch(activePowers.back().type){
            case PowerType::HP: shape.setFillColor(sf::Color::Green); break;
            case PowerType::DoubleShot: shape.setFillColor(sf::Color::Yellow); break;
            case PowerType::HomingShot: shape.setFillColor(sf::Color(128,0,128)); break; // tím
            case PowerType::LaserBeam: shape.setFillColor(sf::Color::Red); break;
        }
    } else shape.setFillColor(sf::Color::Cyan);
}

void Player::render(sf::RenderWindow& window){ window.draw(sprite); }

sf::Vector2f Player::getPos() const { return shape.getPosition(); }
sf::FloatRect Player::getBounds() const { return shape.getGlobalBounds(); }

void Player::takeDamage(int dmg){
    if(!isInvincible()){
        hp -= dmg;
        if(hp<0) hp=0;
        invincibleClock.restart();
    }
}

bool Player::isInvincible() const { return invincibleClock.getElapsedTime().asSeconds() < invincibleDuration; }
void Player::resetHP(){ hp=10; invincibleClock.restart(); }

void Player::addWeapon(PowerType type,float duration){
    activePowers.push_back({type,sf::Clock(),duration});
}

// HP hồi trực tiếp khi ăn power-up, các power khác vẫn lưu duration
void Player::applyPowerUp(PowerType type, float duration){
    if(type == PowerType::HP){
        hp += 2;
        if(hp>10) hp=10;
    } else {
        addWeapon(type,duration);
    }
}
