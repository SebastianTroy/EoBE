#ifndef ENTITYCONTAINERINTERFACE_H
#define ENTITYCONTAINERINTERFACE_H

#include <Shape.h>

#include <memory>
#include <functional>

class Entity;
class EntityContainerInterface {
public:
    virtual ~EntityContainerInterface(){}
    virtual void AddEntity(std::shared_ptr<Entity> entity) = 0;
    virtual void ForEachCollidingWith(const Point& collide, const std::function<void(const std::shared_ptr<Entity>&)>& action) = 0;
    virtual void ForEachCollidingWith(const Line& collide, const std::function<void(const std::shared_ptr<Entity>&)>& action) = 0;
    virtual void ForEachCollidingWith(const Rect& collide, const std::function<void(const std::shared_ptr<Entity>&)>& action) = 0;
    virtual void ForEachCollidingWith(const Circle& collide, const std::function<void(const std::shared_ptr<Entity>&)>& action) = 0;
    virtual void ForEachCollidingWith(const Point& collide, const std::function<void(const Entity&)>& action) const = 0;
    virtual void ForEachCollidingWith(const Line& collide, const std::function<void(const Entity&)>& action) const = 0;
    virtual void ForEachCollidingWith(const Rect& collide, const std::function<void(const Entity&)>& action) const = 0;
    virtual void ForEachCollidingWith(const Circle& collide, const std::function<void(const Entity&)>& action) const = 0;

    template <typename Shape>
    unsigned CountEntities(const Shape& collide) const
    {
        unsigned count = 0;
        ForEachCollidingWith(collide, [&](const Entity&)
        {
            ++count;
        });
        return count;
    }
};

#endif // ENTITYCONTAINERINTERFACE_H
