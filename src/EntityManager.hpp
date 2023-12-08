#ifndef __ENTITYMANAGER__
#define __ENTITYMANAGER__

#include <vector>
#include <map>
#include <memory>
#include <algorithm>

#include "Entity.hpp"

typedef std::vector<std::shared_ptr<Entity>> EntityVec;
typedef std::map<std::string, EntityVec> EntityMap;

class EntityManager
{
public:
    EntityManager();
    void update();
    std::shared_ptr<Entity> addEntity(const std::string &tag);
    EntityVec &getEntities();
    EntityVec &getEntities(const std::string &tag);

private:
    EntityVec m_entities;
    EntityVec m_toAdd;
    EntityMap m_entityMap;
    size_t m_total_entities = 0;
};

#endif