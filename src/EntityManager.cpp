#include "EntityManager.hpp"

bool isDead(std::shared_ptr<Entity> e) { return !e->isAlive(); }

EntityManager::EntityManager()
{
}

std::shared_ptr<Entity> EntityManager::addEntity(const std::string &tag)
{
    auto entity = std::shared_ptr<Entity>(new Entity(tag, m_total_entities++));
    m_toAdd.push_back(entity);
    return entity;
}

void EntityManager::update()
{
    for (auto &e : m_toAdd)
    {
        m_entities.push_back(e);
        m_entityMap[e->getTag()].push_back(e);
    }

    std::vector<std::string> checkedTags;
    for (auto &e : m_entities)
    {
        if (std::find(checkedTags.begin(), checkedTags.end(), e->getTag()) == checkedTags.end())
        {
            m_entityMap[e->getTag()].erase(
                std::remove_if(m_entityMap[e->getTag()].begin(), m_entityMap[e->getTag()].end(), isDead),
                m_entityMap[e->getTag()].end());

            checkedTags.push_back(e->getTag());
        }
    }

    m_entities.erase(
        std::remove_if(m_entities.begin(), m_entities.end(), isDead),
        m_entities.end());

    m_toAdd.clear();
}

EntityVec &EntityManager::getEntities()
{
    return m_entities;
}

EntityVec &EntityManager::getEntities(const std::string &tag)
{
    return m_entityMap[tag];
}