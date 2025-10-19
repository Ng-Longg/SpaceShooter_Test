#include "Enemy.h"
#include <iostream>

sf::Texture Enemy::enemyTexture;
sf::Texture Enemy::bossTexture;
sf::Texture Enemy::tankTexture;
sf::Texture Enemy::speedTexture;

bool Enemy::loadTextures() {
    bool ok1 = enemyTexture.loadFromFile("assets/Enemy.png");
    bool ok2 = bossTexture.loadFromFile("assets/Boss.png");
    bool ok3 = tankTexture.loadFromFile("assets/Tank.png");
    bool ok4 = speedTexture.loadFromFile("assets/Speed.png");

    if (!ok1) std::cerr << "❌ Failed to load Enemy.png\n";
    if (!ok2) std::cerr << "❌ Failed to load Boss.png\n";
    if (!ok3) std::cerr << "❌ Failed to load Tank.png\n";
    if (!ok4) std::cerr << "❌ Failed to load Speed.png\n";

    return ok1 && ok2 && ok3 && ok4;
}

Enemy::Enemy(sf::Vector2f startPos, EnemyType type, sf::Vector2f size)
    : type(type)
{
    switch (type)
    {
        case EnemyType::Normal:
            baseSpeed = 0.25f;
            maxHp = 2;
            shootInterval = 1.5f;
            canShoot = (std::rand() % 100 < 10);
            sprite.setTexture(enemyTexture);
            break;

        case EnemyType::Speed:
            baseSpeed = 0.25f * 1.5f;
            maxHp = 1;
            shootInterval = 1.2f;
            canShoot = false;
            sprite.setTexture(speedTexture);
            break;

        case EnemyType::Tank:
            baseSpeed = 0.25f * 0.5f;
            maxHp = 5;
            shootInterval = 2.0f;
            canShoot = true;
            sprite.setTexture(tankTexture);
            break;

        case EnemyType::Boss:
            baseSpeed = 0.1f;
            maxHp = 100;
            shootInterval = 1.0f;
            canShoot = true;
            sprite.setTexture(bossTexture);
            moveDirection = 1.f; // Boss bắt đầu di chuyển sang phải
            break;
    }

    hp = maxHp;
    // 🔥 Giữ nguyên moveDirection của Boss, các loại khác thì gán 0
    if (type != EnemyType::Boss)
        moveDirection = 0.f;

    sprite.setPosition(startPos);
    sprite.setColor(sf::Color::White);
    setSize(size);
}

void Enemy::setSize(sf::Vector2f newSize) {
    sf::Vector2u texSize = sprite.getTexture()->getSize();
    if (texSize.x > 0 && texSize.y > 0)
        sprite.setScale(newSize.x / texSize.x, newSize.y / texSize.y);
}

void Enemy::update(sf::Vector2f playerPos, std::vector<sf::RectangleShape>& enemyBullets, float speedMultiplier)
{
    float moveSpeed = baseSpeed * speedMultiplier;

    switch (type)
    {
        // --- NORMAL & SPEED: đuổi theo player ---
        case EnemyType::Normal:
        case EnemyType::Speed:
        {
            sf::Vector2f center(sprite.getPosition().x + sprite.getGlobalBounds().width / 2,
                                sprite.getPosition().y + sprite.getGlobalBounds().height / 2);
            sf::Vector2f dir = playerPos - center;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len != 0) dir /= len;

            sprite.move(dir.x * moveSpeed * 0.7f, dir.y * moveSpeed);

            if (canShoot && shootClock.getElapsedTime().asSeconds() >= shootInterval) {
                shootClock.restart();
                sf::RectangleShape bullet(sf::Vector2f(4.f, 12.f));
                bullet.setFillColor(sf::Color::Yellow);
                bullet.setPosition(sprite.getPosition().x + sprite.getGlobalBounds().width / 2 - 2.f,
                                   sprite.getPosition().y + sprite.getGlobalBounds().height);
                enemyBullets.push_back(bullet);
            }
        } break;

        // --- TANK: đuổi player nhưng chậm ---
        case EnemyType::Tank:
        {
            sf::Vector2f center(sprite.getPosition().x + sprite.getGlobalBounds().width / 2,
                                sprite.getPosition().y + sprite.getGlobalBounds().height / 2);
            sf::Vector2f dir = playerPos - center;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len != 0) dir /= len;

            // Tank di chuyển chậm hơn nhiều
            sprite.move(dir.x * moveSpeed * 0.5f, dir.y * moveSpeed * 0.5f);

            if (canShoot && shootClock.getElapsedTime().asSeconds() >= shootInterval) {
                shootClock.restart();
                sf::RectangleShape bullet(sf::Vector2f(6.f, 14.f));
                bullet.setFillColor(sf::Color::Red);
                bullet.setPosition(sprite.getPosition().x + sprite.getGlobalBounds().width / 2 - 3.f,
                                   sprite.getPosition().y + sprite.getGlobalBounds().height);
                enemyBullets.push_back(bullet);
            }
        } break;

        // --- BOSS: di chuyển ngang qua lại + bắn ---
        case EnemyType::Boss:
        {
            sprite.move(moveDirection * moveSpeed * 2.f, 0.f);
            float left = sprite.getPosition().x;
            float right = left + sprite.getGlobalBounds().width;
            const float windowWidth = 1280.f;

            if (left <= 0.f) {
                moveDirection = 1.f;
                sprite.setPosition(0.f, sprite.getPosition().y);
            }
            else if (right >= windowWidth) {
                moveDirection = -1.f;
                sprite.setPosition(windowWidth - sprite.getGlobalBounds().width, sprite.getPosition().y);
            }

            if (shootClock.getElapsedTime().asSeconds() >= shootInterval / speedMultiplier) {
                shootClock.restart();
                float midX = sprite.getPosition().x + sprite.getGlobalBounds().width / 2;
                float midY = sprite.getPosition().y + sprite.getGlobalBounds().height;
                for (float dx : {-40.f, 0.f, 40.f}) {
                    sf::RectangleShape bullet(sf::Vector2f(6.f, 20.f));
                    bullet.setFillColor(sf::Color::Cyan);
                    bullet.setPosition(midX + dx - 3.f, midY);
                    enemyBullets.push_back(bullet);
                }
            }
        } break;
    }
}

void Enemy::render(sf::RenderWindow& window)
{
    window.draw(sprite);

    if (type == EnemyType::Boss) {
        float healthPercent = float(hp) / maxHp;

        sf::RectangleShape back(sf::Vector2f(sprite.getGlobalBounds().width, 6.f));
        back.setFillColor(sf::Color(80, 80, 80));
        back.setPosition(sprite.getPosition().x, sprite.getPosition().y - 10.f);

        sf::RectangleShape bar(sf::Vector2f(sprite.getGlobalBounds().width * healthPercent, 6.f));
        bar.setFillColor(sf::Color::Green);
        bar.setPosition(sprite.getPosition().x, sprite.getPosition().y - 10.f);

        window.draw(back);
        window.draw(bar);
    }
}

sf::FloatRect Enemy::getBounds() const { return sprite.getGlobalBounds(); }
bool Enemy::isDead() const { return hp <= 0; }
void Enemy::takeDamage(int dmg) { hp = std::max(0, hp - dmg); }
EnemyType Enemy::getType() const { return type; }
int Enemy::getHp() const { return hp; }
int Enemy::getMaxHp() const { return maxHp; }
