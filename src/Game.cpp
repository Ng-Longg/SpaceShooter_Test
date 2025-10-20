#include "Game.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

// ===================== CONSTRUCTOR =====================
Game::Game()
    : window(sf::VideoMode(1280, 720), "Space Shooter - Boss & HighScore"),
      score(0), highScore(0), isGameOver(false), bossSpawned(false),
      difficultyLevel(1), enemySpawnInterval(1.0f), enemySpeedMultiplier(1.0f),
      screenShakeMagnitude(0.f), screenShakeDuration(0.f)
{
    if (!Enemy::loadTextures())
        std::cerr << "❌ Error: Cannot load enemy/boss textures!\n";

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Load high score
    std::ifstream inFile("highscore.txt");
    if (inFile.is_open()) { inFile >> highScore; inFile.close(); }

    // Load font
    if (!font.loadFromFile("arial.ttf"))
        std::cerr << "⚠️ Warning: cannot load font\n";

    // Text setup
    scoreText.setFont(font); scoreText.setCharacterSize(22); scoreText.setFillColor(sf::Color::White); scoreText.setPosition(10,8);
    highScoreText.setFont(font); highScoreText.setCharacterSize(22); highScoreText.setFillColor(sf::Color::Yellow); highScoreText.setPosition(10,40);
    hpText.setFont(font); hpText.setCharacterSize(22); hpText.setFillColor(sf::Color::Red); hpText.setPosition(10,70);
    gameOverText.setFont(font); gameOverText.setCharacterSize(36); gameOverText.setFillColor(sf::Color::Red);
    gameOverText.setString("GAME OVER - Press R to Restart"); gameOverText.setPosition(350,320);

    // Load background
    if (!backgroundTexture.loadFromFile("assets/Background.png"))
        std::cerr << "⚠️ Warning: Cannot load background image!\n";
    backgroundSprite.setTexture(backgroundTexture);
    backgroundSprite.setScale(
        float(window.getSize().x) / backgroundTexture.getSize().x,
        float(window.getSize().y) / backgroundTexture.getSize().y
    );

    // ===== AUDIO SETUP =====
    // Background music (streamed) - loop forever
    if (!bgMusic.openFromFile("assets/musicbg.ogg")) {
        std::cerr << "⚠️ Warning: Cannot load background music (assets/musicbg.ogg)!\n";
    } else {
        bgMusic.setLoop(true);
        bgMusic.setVolume(50.f); // default volume
        bgMusic.play();
    }

    // Explosion sound (short SFX)
    if (!explosionBuffer.loadFromFile("assets/explosion.wav")) {
        std::cerr << "⚠️ Warning: Cannot load explosion sound (assets/explosion.wav)!\n";
    } else {
        explosionSound.setBuffer(explosionBuffer);
        explosionSound.setVolume(80.f);
    }

    shootTimer.restart();
    enemySpawnTimer.restart();
    difficultyClock.restart();
}

// ===================== MAIN LOOP =====================
void Game::run() {
    while (window.isOpen()) {
        processEvents();
        update();
        render();
    }
}

// ===================== EVENTS =====================
void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed)
            window.close();
    }
    if (isGameOver && sf::Keyboard::isKeyPressed(sf::Keyboard::R))
        resetGame();

    // Toggle music on/off with M (optional convenience)
    static bool mPressedLast = false;
    bool mPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::M);
    if (mPressed && !mPressedLast) {
        if (bgMusic.getStatus() == sf::Music::Playing) bgMusic.pause();
        else bgMusic.play();
    }
    mPressedLast = mPressed;
}

// ===================== UPDATE =====================
void Game::update() {
    if (isGameOver) return;

    // ===================== INCREASE DIFFICULTY =====================
    if (difficultyClock.getElapsedTime().asSeconds() > 10.f) {
        increaseDifficulty();
        difficultyClock.restart();
    }

    player.update(window);
    sf::Vector2f playerCenter(player.getBounds().left + player.getBounds().width / 2.f,
                               player.getBounds().top + player.getBounds().height / 2.f);

    // ===================== PLAYER SHOOT =====================
    if (sf::Mouse::isButtonPressed(sf::Mouse::Left) && shootTimer.getElapsedTime().asMilliseconds() > 200) {
        const auto& activePowers = player.getActivePowers();

        if (activePowers.empty()) {
            bullets.emplace_back(playerCenter, PowerType::DoubleShot);
        } else {
            for (const auto& p : activePowers) {
                switch (p.type) {
                    case PowerType::DoubleShot:
                        bullets.emplace_back(playerCenter + sf::Vector2f(-10,0), PowerType::DoubleShot);
                        bullets.emplace_back(playerCenter + sf::Vector2f(10,0), PowerType::DoubleShot);
                        break;
                    case PowerType::HomingShot:
                        bullets.emplace_back(playerCenter, PowerType::HomingShot);
                        break;
                    case PowerType::LaserBeam:
                        bullets.emplace_back(playerCenter, PowerType::LaserBeam);
                        break;
                    case PowerType::HP:
                        break;
                }
            }
        }
        shootTimer.restart();
    }

    // ===================== UPDATE BULLETS =====================
    for (auto& b : bullets)
        (b.getType() == PowerType::LaserBeam) ? b.followPlayer(playerCenter) : b.update(enemies);

    // ===================== SPAWN BOSS =====================
    static sf::Clock bossSpawnClock;
    if (!bossSpawned && bossSpawnClock.getElapsedTime().asSeconds() > 10.f) {
        if (std::rand() % 100 < 2) {
            float x = 50 + std::rand() % (window.getSize().x - 200);
            sf::Vector2f bossSize(
                Enemy::getBossTexture().getSize().x * 0.5f,
                Enemy::getBossTexture().getSize().y * 0.5f
            );
            enemies.emplace_back(sf::Vector2f(x, 60.f), EnemyType::Boss, bossSize);
            bossSpawned = true;
            bossSpawnClock.restart();
            startScreenShake(0.8f, 10.f);
        }
    }

    // ===================== SPAWN NORMAL / SPECIAL ENEMY =====================
    if (enemySpawnTimer.getElapsedTime().asSeconds() > enemySpawnInterval) {
        spawnEnemy();
        enemySpawnTimer.restart();
    }

    // ===================== UPDATE ENEMIES =====================
    for (auto& e : enemies)
        e.update(playerCenter, enemyBullets, enemySpeedMultiplier);

    // Move enemy bullets
    for (auto& eb : enemyBullets)
        eb.move(0.f, 0.3f * enemySpeedMultiplier);

    // ===================== ENEMY BULLETS VS PLAYER =====================
    // Duyệt ngược để xóa an toàn khi trúng
    for (int i = int(enemyBullets.size()) - 1; i >= 0; --i) {
        if (enemyBullets[i].getGlobalBounds().intersects(player.getBounds())) {
            if (!player.isInvincible()) {
                player.takeDamage(1);
                createExplosion(playerCenter, sf::Color::Red, 15.f);
                startScreenShake(0.3f, 8.f);
            }

            // Xóa viên đạn vừa chạm
            enemyBullets.erase(enemyBullets.begin() + i);

            // Nếu máu <= 0 => game over
            if (player.getHP() <= 0) {
                isGameOver = true;
                saveHighScore();
                return; // thoát update() để tránh xử lý tiếp
            }
        }
    }

    // ===================== COLLISION BULLET/ENEMY =====================
    std::vector<int> bulletsToRemove;
    for (int ei = int(enemies.size()) - 1; ei >= 0; --ei) {
        for (int bi = int(bullets.size()) - 1; bi >= 0; --bi) {
            if (enemies[ei].getBounds().intersects(bullets[bi].getBounds())) {
                PowerType bt = bullets[bi].getType();
                if (bt == PowerType::LaserBeam) {
                    if (bullets[bi].damageClock.getElapsedTime().asSeconds() >= 0.3f) {
                        enemies[ei].takeDamage(3);
                        bullets[bi].damageClock.restart();
                    }
                } else {
                    enemies[ei].takeDamage(1);
                    bulletsToRemove.push_back(bi);
                }

                if (enemies[ei].isDead()) {
                    bool isBoss = (enemies[ei].getType() == EnemyType::Boss);
                    createExplosion(enemies[ei].getBounds().getPosition(),
                                    isBoss ? sf::Color::Yellow : sf::Color::White,
                                    isBoss ? 40.f : 15.f);
                    score += isBoss ? 100 : 10;
                    if (score > highScore) highScore = score;

                    if (isBoss) {
                        bossSpawned = false;
                        bossSpawnClock.restart();
                        startScreenShake(0.8f, 12.f);
                        // --- Play explosion SFX only when boss dies ---
                        if (explosionBuffer.getSampleCount() > 0) {
                            explosionSound.play();
                        }
                    }

                    if (std::rand() % 5 == 0) {
                        PowerType pt;
                        int r = std::rand() % 4;
                        switch (r) {
                            case 0: pt = PowerType::HP; break;
                            case 1: pt = PowerType::DoubleShot; break;
                            case 2: pt = PowerType::HomingShot; break;
                            case 3: pt = PowerType::LaserBeam; break;
                        }
                        powerUps.emplace_back(enemies[ei].getBounds().getPosition(), pt);
                    }

                    enemies.erase(enemies.begin() + ei);
                    break;
                }
            }
        }
    }

    std::sort(bulletsToRemove.rbegin(), bulletsToRemove.rend());
    for (int bi : bulletsToRemove)
        if (bi >= 0 && bi < bullets.size()) bullets.erase(bullets.begin() + bi);

    bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
        [](Bullet& b) { return b.outOfWindow(); }), bullets.end());

    enemyBullets.erase(std::remove_if(enemyBullets.begin(), enemyBullets.end(),
        [&](sf::RectangleShape& eb) { return eb.getPosition().y > window.getSize().y; }), enemyBullets.end());

    // ===================== POWER-UPS =====================
    for (int i = int(powerUps.size()) - 1; i >= 0; --i) {
        if (powerUps[i].getBounds().intersects(player.getBounds())) {
            player.applyPowerUp(powerUps[i].getType(),5.f);
            powerUps.erase(powerUps.begin() + i);
        }
    }

    for (auto& p : powerUps) p.update();
    powerUps.erase(std::remove_if(powerUps.begin(), powerUps.end(),
        [](PowerUp& p) { return p.outOfWindow(); }), powerUps.end());

    // ===================== ENEMY COLLIDES PLAYER =====================
    for (int i = int(enemies.size()) - 1; i >= 0; --i) {
        if (enemies[i].getBounds().intersects(player.getBounds())) {
            if (enemies[i].getType() == EnemyType::Boss) {
                isGameOver = true;
                startScreenShake(1.f, 14.f);
                createExplosion(playerCenter, sf::Color::Red, 50.f);
                saveHighScore();
                return;
            } else {
                player.takeDamage(1);
                createExplosion(playerCenter, sf::Color::Red, 20.f);
                enemies.erase(enemies.begin() + i);
                if (player.getHP() <= 0) { isGameOver = true; startScreenShake(1.f, 12.f); saveHighScore(); return; }
            }
        }
    }

    updateScreenShake();
    updateExplosions();

    scoreText.setString("Score: " + std::to_string(score));
    highScoreText.setString("High Score: " + std::to_string(highScore));
    hpText.setString("HP: " + std::to_string(player.getHP()));
}

// ===================== RENDER =====================
void Game::render() {
    window.clear();
    sf::View view = window.getDefaultView();
    view.move(screenShakeOffset);
    window.setView(view);

    window.draw(backgroundSprite);
    player.render(window);
    for (auto& b : bullets) b.render(window);
    for (auto& e : enemies) e.render(window);
    for (auto& eb : enemyBullets) window.draw(eb);
    for (auto& p : powerUps) p.render(window);
    for (auto& e : explosions) window.draw(e);

    window.draw(scoreText);
    window.draw(highScoreText);
    window.draw(hpText);
    if (isGameOver) window.draw(gameOverText);

    window.display();
}

// ===================== SPAWN ENEMY =====================
void Game::spawnEnemy() {
    float x = float(std::rand() % (window.getSize().x - 60));

    int r = std::rand() % 100;
    EnemyType type = EnemyType::Normal;

    if (r < 70) type = EnemyType::Normal;      // 70% thường
    else if (r < 90) type = EnemyType::Speed;  // 20% nhanh
    else type = EnemyType::Tank;               // 10% trâu

    sf::Vector2f size;
    switch (type) {
        case EnemyType::Normal:
            size = sf::Vector2f(Enemy::getEnemyTexture().getSize().x * 0.5f,
                                Enemy::getEnemyTexture().getSize().y * 0.5f);
            break;
        case EnemyType::Speed:
            size = sf::Vector2f(Enemy::getSpeedTexture().getSize().x * 0.5f,
                                Enemy::getSpeedTexture().getSize().y * 0.5f);
            break;
        case EnemyType::Tank:
            size = sf::Vector2f(Enemy::getTankTexture().getSize().x * 0.3f,
                                Enemy::getTankTexture().getSize().y * 0.3f);
            break;
        default: break;
    }

    enemies.emplace_back(sf::Vector2f(x, -60.f), type, size);
}

// ===================== RESET GAME =====================
void Game::resetGame() {
    enemies.clear(); bullets.clear(); enemyBullets.clear(); powerUps.clear(); explosions.clear();
    score = 0; isGameOver = false; bossSpawned = false;
    difficultyLevel = 1; enemySpawnInterval = 1.0f; enemySpeedMultiplier = 1.0f;
    enemySpawnTimer.restart(); shootTimer.restart(); difficultyClock.restart();
    player.resetHP();
    // keep bgMusic playing (or paused) as-is
}

// ===================== HIGH SCORE =====================
void Game::saveHighScore() {
    std::ofstream outFile("highscore.txt");
    if (outFile.is_open()) { outFile << highScore; outFile.close(); }
}

// ===================== DIFFICULTY =====================
void Game::increaseDifficulty() {
    difficultyLevel++;
    enemySpawnInterval = std::max(0.3f, enemySpawnInterval - 0.1f);
    enemySpeedMultiplier += 0.1f;
}

// ===================== SCREEN SHAKE =====================
void Game::startScreenShake(float duration, float magnitude) {
    screenShakeDuration = duration;
    screenShakeMagnitude = magnitude;
    screenShakeClock.restart();
}

void Game::updateScreenShake() {
    if (screenShakeClock.getElapsedTime().asSeconds() < screenShakeDuration) {
        float t = screenShakeClock.getElapsedTime().asSeconds();
        float decay = 1.f - (t / screenShakeDuration);
        screenShakeOffset.x = (std::rand()%100/100.f-0.5f)*2*screenShakeMagnitude*decay;
        screenShakeOffset.y = (std::rand()%100/100.f-0.5f)*2*screenShakeMagnitude*decay;
    } else screenShakeOffset = {0.f,0.f};
}

// ===================== EXPLOSIONS =====================
void Game::createExplosion(sf::Vector2f pos, sf::Color color, float size) {
    sf::CircleShape explosion(size);
    explosion.setOrigin(size,size);
    explosion.setPosition(pos);
    explosion.setFillColor(color);
    explosion.setOutlineThickness(2.f);
    explosion.setOutlineColor(sf::Color::White);
    explosions.push_back(explosion);
    explosionTimers.emplace_back();
}

void Game::updateExplosions() {
    for (int i=int(explosions.size())-1;i>=0;--i){
        float elapsed = explosionTimers[i].getElapsedTime().asSeconds();
        float scale = 1.f + elapsed*2.f;
        explosions[i].setScale(scale,scale);
        sf::Color c = explosions[i].getFillColor();
        if(c.a>5) c.a = static_cast<sf::Uint8>(255*(1.f-elapsed/0.5f));
        explosions[i].setFillColor(c);
        if(elapsed>0.5f){ explosions.erase(explosions.begin()+i); explosionTimers.erase(explosionTimers.begin()+i);}
    }
}
