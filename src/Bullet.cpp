#include "Bullet.h"
#include <cmath>
#include <limits>

Bullet::Bullet(sf::Vector2f startPos, PowerType type)
{
    bulletType = type;
    speed = 0.3f;
    velocity = {0.f, -speed};

    maxLifeTime = 0.3f; 
    lifeClock.restart();
    damageClock.restart();

    if(type == PowerType::LaserBeam){
        shape.setSize(sf::Vector2f(8.f,800.f));
        shape.setFillColor(sf::Color(255,0,0,180));
        shape.setOrigin(shape.getSize().x/2.f, shape.getSize().y);
        shape.setPosition(startPos.x,startPos.y);
    } else {
        shape.setSize(sf::Vector2f(8.f,16.f));
        if(type == PowerType::HomingShot)
            shape.setFillColor(sf::Color(128,0,128)); // tím
        else
            shape.setFillColor(sf::Color::Yellow);

        shape.setPosition(startPos.x - shape.getSize().x/2.f, startPos.y - shape.getSize().y - 4.f);
    }
}

void Bullet::update(std::vector<Enemy>& enemies){
    if(bulletType == PowerType::LaserBeam) return;

    if(bulletType == PowerType::HomingShot && !enemies.empty()){
        Enemy* target = nullptr;
        float minDist = std::numeric_limits<float>::infinity();
        sf::Vector2f pos = shape.getPosition() + sf::Vector2f(shape.getSize().x/2.f,shape.getSize().y/2.f);

        for(auto& e: enemies){
            sf::FloatRect eb = e.getBounds();
            sf::Vector2f eCenter(eb.left+eb.width/2.f, eb.top+eb.height/2.f);
            float dx = eCenter.x - pos.x;
            float dy = eCenter.y - pos.y;
            float dist = std::sqrt(dx*dx + dy*dy);
            if(dist < minDist){
                minDist = dist;
                target = &e;
            }
        }

        if(target){
            sf::FloatRect tb = target->getBounds();
            sf::Vector2f targetCenter(tb.left+tb.width/2.f,tb.top+tb.height/2.f);
            sf::Vector2f dir = targetCenter - (shape.getPosition()+sf::Vector2f(shape.getSize().x/2.f,shape.getSize().y/2.f));
            float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
            if(len!=0) dir/=len;
            velocity = dir*speed;
        }
    }

    shape.move(velocity);
}

void Bullet::followPlayer(const sf::Vector2f& playerCenter){
    if(bulletType == PowerType::LaserBeam) shape.setPosition(playerCenter.x,playerCenter.y);
}

void Bullet::render(sf::RenderWindow& window){ window.draw(shape); }
sf::FloatRect Bullet::getBounds() const { return shape.getGlobalBounds(); }
bool Bullet::outOfWindow() const {
    if(bulletType == PowerType::LaserBeam) return lifeClock.getElapsedTime().asSeconds() > maxLifeTime;
    return (shape.getPosition().y+shape.getSize().y) < 0.f;
}
PowerType Bullet::getType() const { return bulletType; }
