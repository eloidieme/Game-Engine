#ifndef __ENTITY__
#define __ENTITY__

#include "Components.hpp"

#include <string>
#include <memory>

class Entity
{
public:
    friend class EntityManager;
    std::string getTag() { return m_tag; }
    bool isAlive() { return m_alive; }
    void destroy() { m_alive = false; }

    // Components (pointers)
    std::shared_ptr<CTransform> cTransform;
    std::shared_ptr<CShape> cShape;
    std::shared_ptr<CCollision> cCollision;
    std::shared_ptr<CInput> cInput;
    std::shared_ptr<CScore> cScore;
    std::shared_ptr<CLifespan> cLifespan;

private:
    Entity(const std::string &tag, size_t id);
    const size_t m_id = 0;
    const std::string m_tag = "Default";
    bool m_alive = true;
};

#endif