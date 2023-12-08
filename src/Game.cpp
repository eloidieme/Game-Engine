#include "Game.hpp"

#include <cmath>
#include <iostream>
#include <fstream>

const int BTYE_RANGE{256};
const int SCORE_MULTIPLIER{100};

Game::Game(const std::string &config)
{
    init(config);
}

void Game::init(const std::string &path)
{
    std::ifstream fin(path);
    std::string configSpecifier;

    // Window
    int wWidth, wHeight, wFrameLimit, wFullScreen;

    // Font
    std::string fFile;
    int fSize, fColorR, fColorG, fColorB;

    while (fin >> configSpecifier)
    {
        if (configSpecifier == "Window")
        {
            fin >> wWidth >> wHeight >> wFrameLimit >> wFullScreen;
        }

        if (configSpecifier == "Font")
        {
            fin >> fFile >> fSize >> fColorR >> fColorG >> fColorB;
        }

        if (configSpecifier == "Player")
        {
            fin >> m_playerConfig.SR >> m_playerConfig.CR >> m_playerConfig.S >> m_playerConfig.FR >> m_playerConfig.FG >> m_playerConfig.FB >> m_playerConfig.OR >> m_playerConfig.OG >> m_playerConfig.OB >> m_playerConfig.OT >> m_playerConfig.V;
        }

        if (configSpecifier == "Enemy")
        {
            fin >> m_enemyConfig.SR >> m_enemyConfig.CR >> m_enemyConfig.SMIN >> m_enemyConfig.SMAX >> m_enemyConfig.OR >> m_enemyConfig.OG >> m_enemyConfig.OB >> m_enemyConfig.OT >> m_enemyConfig.VMIN >> m_enemyConfig.VMAX >> m_enemyConfig.L >> m_enemyConfig.SI;
        }

        if (configSpecifier == "Bullet")
        {
            fin >> m_bulletConfig.SR >> m_bulletConfig.CR >> m_bulletConfig.S >> m_bulletConfig.FR >> m_bulletConfig.FG >> m_bulletConfig.FB >> m_bulletConfig.OR >> m_bulletConfig.OG >> m_bulletConfig.OB >> m_bulletConfig.OT >> m_bulletConfig.V >> m_bulletConfig.L;
        }
    }

    // fullscreen broken on macOS
    m_window.create(wFullScreen ? sf::VideoMode::getFullscreenModes()[0] : sf::VideoMode(wWidth, wHeight), "Geometry Wars", wFullScreen ? sf::Style::Fullscreen : sf::Style::Default);
    m_window.setFramerateLimit(wFrameLimit);

    std::cout << sf::VideoMode::getFullscreenModes().size() << '\n';

    if (!m_font.loadFromFile(fFile))
    {
        std::cout << "Unable to load font !" << '\n';
        exit(-1);
    }

    m_text.setFont(m_font);
    m_text.setCharacterSize(fSize);
    m_text.setFillColor(sf::Color(fColorR, fColorG, fColorB));
    m_text.setString("Score " + std::to_string(m_score));
    m_text.setPosition({10U, 10U});

    spawnPlayer();
}

void Game::run()
{
    while (m_running)
    {
        m_entities.update();

        if (!m_paused)
        {
            sEnemySpawner();
            sMovement();
            sLifespan();
            sCollision();
            m_currentFrame++;
        }

        sUserInput();
        sRender();
    }
}

void Game::setPaused(bool paused)
{

    m_paused = paused;
}

void Game::spawnPlayer()
{
    auto entity = m_entities.addEntity("player");

    Vec2D starting_pos(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f);

    entity->cTransform = std::make_shared<CTransform>(starting_pos, Vec2D(0.0f, 0.0f), 0.0f);
    entity->cShape = std::make_shared<CShape>(m_playerConfig.SR, m_playerConfig.V, sf::Color(m_playerConfig.FR, m_playerConfig.FG, m_playerConfig.FB), sf::Color(m_playerConfig.OR, m_playerConfig.OG, m_playerConfig.OB), m_playerConfig.OT);
    entity->cInput = std::make_shared<CInput>();
    entity->cCollision = std::make_shared<CCollision>(m_playerConfig.CR);

    m_player = entity;
}

void Game::spawnEnemy()
{
    srand(time(NULL));

    float posX = m_enemyConfig.SR + rand() % (m_window.getSize().x - m_enemyConfig.SR - m_enemyConfig.SR);
    float posY = m_enemyConfig.SR + rand() % (m_window.getSize().y - m_enemyConfig.SR - m_enemyConfig.SR);

    Vec2D starting_vel(-10 + rand() % 21, -10 + rand() % 21); // create a "random" vector of coords in range[-10, 10]
    starting_vel.normalize_emplace();                         // normalize this vector

    float speed = m_enemyConfig.SMIN + rand() % static_cast<int>(m_enemyConfig.SMAX - m_enemyConfig.SMIN + 1);
    int vertices = m_enemyConfig.VMIN + rand() % (m_enemyConfig.VMAX - m_enemyConfig.VMIN + 1);

    int r = rand() % BTYE_RANGE;
    int g = rand() % BTYE_RANGE;
    int b = rand() % BTYE_RANGE;

    auto new_enemy = m_entities.addEntity("enemy");

    new_enemy->cTransform = std::make_shared<CTransform>(Vec2D(posX, posY), speed * starting_vel, 0.0f);

    new_enemy->cShape = std::make_shared<CShape>(m_enemyConfig.SR, vertices, sf::Color(r, g, b), sf::Color(m_enemyConfig.OR, m_enemyConfig.OG, m_enemyConfig.OB), m_enemyConfig.OT);
    new_enemy->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR);
    new_enemy->cScore = std::make_shared<CScore>(vertices * SCORE_MULTIPLIER);

    m_lastEnemySpawnTime = m_currentFrame;
}

void Game::spawnSmallEnemies(std::shared_ptr<Entity> entity)
{
    // TODO: spawn small enemies at the location of the input enemy
    for (int i = 0; i < entity->cShape->circle.getPointCount(); i++)
    {
        auto e = m_entities.addEntity("smallenemy");

        e->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, Vec2D(cosf(i * 360 / entity->cShape->circle.getPointCount() * M_PI / 180.0), sinf(i * 360 / entity->cShape->circle.getPointCount() * M_PI / 180.0)), 0.0f);
        e->cShape = std::make_shared<CShape>(entity->cShape->circle.getRadius() / 2, entity->cShape->circle.getPointCount(), entity->cShape->circle.getFillColor(), entity->cShape->circle.getOutlineColor(), entity->cShape->circle.getOutlineThickness());
        e->cCollision = std::make_shared<CCollision>(m_enemyConfig.CR / 2);
        e->cLifespan = std::make_shared<CLifespan>(m_enemyConfig.L);
        e->cScore = std::make_shared<CScore>(2 * entity->cShape->circle.getPointCount() * SCORE_MULTIPLIER);
    }
}
void Game::spawnBullet(std::shared_ptr<Entity> entity, const Vec2D &mousePos)
{
    auto bullet = m_entities.addEntity("bullet");

    bullet->cTransform = std::make_shared<CTransform>(entity->cTransform->pos, m_bulletConfig.S * (mousePos - entity->cTransform->pos).normalize(), 0.0f);
    bullet->cShape = std::make_shared<CShape>(m_bulletConfig.SR, m_bulletConfig.V, sf::Color(m_bulletConfig.FR, m_bulletConfig.FG, m_bulletConfig.FB), sf::Color(m_bulletConfig.OR, m_bulletConfig.OG, m_bulletConfig.OB), m_bulletConfig.OT);
    bullet->cCollision = std::make_shared<CCollision>(m_bulletConfig.CR);
    bullet->cLifespan = std::make_shared<CLifespan>(m_bulletConfig.L);
}
void Game::spawnSpecialWeapon(std::shared_ptr<Entity> entity)
{
    // TODO: implement special weapon
}

// Systems
void Game::sMovement()
{
    m_player->cTransform->velocity = Vec2D(0.0f, 0.0f);

    if (m_player->cInput->up && m_player->cTransform->pos.getY() > m_player->cShape->circle.getRadius() + m_player->cShape->circle.getOutlineThickness())
        m_player->cTransform->velocity += m_playerConfig.S * Vec2D(0.0f, -1.0f);
    if (m_player->cInput->down && m_player->cTransform->pos.getY() < m_window.getSize().y - (m_player->cShape->circle.getRadius() + m_player->cShape->circle.getOutlineThickness()))
        m_player->cTransform->velocity += m_playerConfig.S * Vec2D(0.0f, 1.0f);
    if (m_player->cInput->right && m_player->cTransform->pos.getX() < m_window.getSize().x - (m_player->cShape->circle.getRadius() + m_player->cShape->circle.getOutlineThickness()))
        m_player->cTransform->velocity += m_playerConfig.S * Vec2D(1.0f, 0.0f);
    if (m_player->cInput->left && m_player->cTransform->pos.getX() > m_player->cShape->circle.getRadius() + m_player->cShape->circle.getOutlineThickness())
        m_player->cTransform->velocity += m_playerConfig.S * Vec2D(-1.0f, 0.0f);

    for (auto &e : m_entities.getEntities("enemy"))
    {
        if (e->cTransform->pos.getX() - (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) <= 0)
        {
            e->cTransform->velocity = Vec2D(-e->cTransform->velocity.getX(), e->cTransform->velocity.getY());
        }
        if (e->cTransform->pos.getX() + (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) >= m_window.getSize().x)
        {
            e->cTransform->velocity = Vec2D(-e->cTransform->velocity.getX(), e->cTransform->velocity.getY());
        }
        if (e->cTransform->pos.getY() - (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) <= 0)
        {
            e->cTransform->velocity = Vec2D(e->cTransform->velocity.getX(), -e->cTransform->velocity.getY());
        }
        if (e->cTransform->pos.getY() + (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) >= m_window.getSize().y)
        {
            e->cTransform->velocity = Vec2D(e->cTransform->velocity.getX(), -e->cTransform->velocity.getY());
        }
    }

    for (auto &e : m_entities.getEntities("smallenemy"))
    {
        if (e->cTransform->pos.getX() - (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) <= 0)
        {
            e->cTransform->velocity = Vec2D(-e->cTransform->velocity.getX(), e->cTransform->velocity.getY());
        }
        if (e->cTransform->pos.getX() + (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) >= m_window.getSize().x)
        {
            e->cTransform->velocity = Vec2D(-e->cTransform->velocity.getX(), e->cTransform->velocity.getY());
        }
        if (e->cTransform->pos.getY() - (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) <= 0)
        {
            e->cTransform->velocity = Vec2D(e->cTransform->velocity.getX(), -e->cTransform->velocity.getY());
        }
        if (e->cTransform->pos.getY() + (e->cShape->circle.getRadius() + e->cShape->circle.getOutlineThickness()) >= m_window.getSize().y)
        {
            e->cTransform->velocity = Vec2D(e->cTransform->velocity.getX(), -e->cTransform->velocity.getY());
        }
    }

    for (auto &e : m_entities.getEntities())
    {
        e->cTransform->pos += e->cTransform->velocity;
    }
}
void Game::sUserInput()
{
    sf::Event event;
    while (m_window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
        {
            m_running = false;
        }

        if (event.type == sf::Event::KeyPressed)
        {
            switch (event.key.scancode)
            {
            case sf::Keyboard::Scan::Right:
                m_player->cInput->right = true;
                break;

            case sf::Keyboard::Scan::Left:
                m_player->cInput->left = true;
                break;

            case sf::Keyboard::Scan::Up:
                m_player->cInput->up = true;
                break;

            case sf::Keyboard::Scan::Down:
                m_player->cInput->down = true;
                break;

            case sf::Keyboard::Scan::Escape:
                setPaused(!m_paused);
                break;

            default:
                break;
            }
        }

        if (event.type == sf::Event::KeyReleased)
        {
            switch (event.key.scancode)
            {
            case sf::Keyboard::Scan::Right:
                m_player->cInput->right = false;
                break;

            case sf::Keyboard::Scan::Left:
                m_player->cInput->left = false;
                break;

            case sf::Keyboard::Scan::Up:
                m_player->cInput->up = false;
                break;

            case sf::Keyboard::Scan::Down:
                m_player->cInput->down = false;
                break;

            default:
                break;
            }
        }

        if (event.type == sf::Event::MouseButtonPressed)
        {
            if (event.mouseButton.button == sf::Mouse::Left)
            {
                spawnBullet(m_player, Vec2D(event.mouseButton.x, event.mouseButton.y));
            }
        }
    }
}
void Game::sLifespan()
{
    for (auto &e : m_entities.getEntities())
    {
        if (e->cLifespan)
        {
            if (e->cLifespan->remaining > 0)
            {
                if (e->isAlive())
                {
                    e->cShape->circle.setFillColor(sf::Color(e->cShape->circle.getFillColor().r, e->cShape->circle.getFillColor().g, e->cShape->circle.getFillColor().b, (1.0f * e->cLifespan->remaining / e->cLifespan->total) * 255));
                }
                e->cLifespan->remaining -= 1;
            }
            else
            {
                e->destroy();
            }
        }
    }
}
void Game::sRender()
{
    m_window.clear();

    for (auto &e : m_entities.getEntities())
    {
        e->cShape->circle.setPosition(e->cTransform->pos.getX(), e->cTransform->pos.getY());

        e->cTransform->angle += 1.0f;
        e->cShape->circle.setRotation(e->cTransform->angle);

        m_window.draw(e->cShape->circle);
    }
    m_text.setString("Score " + std::to_string(m_score));
    m_window.draw(m_text);

    m_window.display();
}
void Game::sEnemySpawner()
{
    if (m_currentFrame - m_lastEnemySpawnTime == m_enemyConfig.SI)
    {
        spawnEnemy();
    }
}
void Game::sCollision()
{
    for (auto &e : m_entities.getEntities("enemy"))
    {
        if ((m_player->cTransform->pos - e->cTransform->pos).length() < e->cCollision->radius + m_player->cCollision->radius)
        {
            m_player->cTransform->pos = Vec2D(m_window.getSize().x / 2.0f, m_window.getSize().y / 2.0f);
            e->destroy();
            spawnSmallEnemies(e);
            m_score = 0;
        }

        for (auto &b : m_entities.getEntities("bullet"))
        {
            if ((b->cTransform->pos - e->cTransform->pos).length() < e->cCollision->radius + b->cCollision->radius)
            {
                e->destroy();
                b->destroy();
                spawnSmallEnemies(e);
                m_score += e->cScore->score;
            }
        }
    }

    for (auto &e : m_entities.getEntities("smallenemy"))
    {
        if ((m_player->cTransform->pos - e->cTransform->pos).length() < e->cCollision->radius + m_player->cCollision->radius)
        {
            e->destroy();
        }
    }
}