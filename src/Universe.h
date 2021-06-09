#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "DrawSettings.h"
#include "Spawner.h"
#include "LineGraph.h"
#include "Entity.h"
#include "EntityContainerInterface.h"
#include "UniverseParameters.h"
#include "PropertyTableModel.h"

#include <Energy.h>
#include <AutoClearingContainer.h>
#include <QuadTree.h>
#include <ChromeTracing.h>

#include <QTimer>
#include <QPainter>

#include <iomanip>
#include <functional>
#include <math.h>

class Universe : public EntityContainerInterface {
public:
    Universe(Rect startingQuad);

    void SetEntityTargetPerQuad(uint64_t target, uint64_t leeway);

    void AddEntity(std::shared_ptr<Entity> entity) override { rootNode_.Insert(entity); }
    void ForEachCollidingWith(const Point& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) override final;
    void ForEachCollidingWith(const Line& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) override final;
    void ForEachCollidingWith(const Rect& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) override final;
    void ForEachCollidingWith(const Circle& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action) override final;
    void ForEachCollidingWith(const Point& collide, const std::function<void (const Entity&)>& action) const override final;
    void ForEachCollidingWith(const Line& collide, const std::function<void (const Entity&)>& action) const override final;
    void ForEachCollidingWith(const Rect& collide, const std::function<void (const Entity&)>& action) const override final;
    void ForEachCollidingWith(const Circle& collide, const std::function<void (const Entity&)>& action) const override final;

    std::shared_ptr<Entity> PickEntity(const Point& location, bool remove);
    void ClearAllEntities() { rootNode_.Clear(); }
    template <typename... T>
    void ClearAllEntitiesOfType()
    {
        TRACE_FUNC()
        rootNode_.RemoveIf([](const Entity& item) -> bool
        {
            return (dynamic_cast<const T*>(&item) || ...);
        });
    }
    void ForEach(const std::function<void (const Entity& e)>& action) const
    {
        TRACE_FUNC()
        rootNode_.ForEachItem(Tril::ConstQuadTreeIterator<Entity>([=](const Entity& e){action(e);}));
    }
    void ForEach(const std::function<void (std::shared_ptr<Entity> e)>& action)
    {
        TRACE_FUNC()
        rootNode_.ForEachItem(Tril::QuadTreeIterator<Entity>([=](std::shared_ptr<Entity> e){action(e);}));
    }

    void AddSpawner(const std::shared_ptr<Spawner>& spawner) { spawners_.push_back(spawner); }
    std::shared_ptr<Spawner> PickSpawner(const Point& location, bool remove);
    void ClearAllSpawners() { spawners_.clear(); }

    UniverseParameters& GetParameters() { return params_; }
    const EntityContainerInterface& GetEntityContainer() const { return *this; }

    /**
     * @brief The Task system allows clients to have code run each tick, without
     * the Universe needing to know implementation specifics. For example,
     * graphs can use tasks to collect the data they wish to display.
     *
     * @param task The code to be run each tick.
     *
     * @return A Handle that defines the lifetime of the task. When no copies of
     * the handle exist, the task is removed from the list
     */
    [[nodiscard]] Tril::Handle AddTask(std::function<void(uint64_t tick)>&& task);


    void Tick();
    void Draw(QPainter& painter, const DrawSettings& options, const Rect& drawArea);

    std::vector<Property> GetProperties() const;

private:
    Tril::QuadTree<Entity> rootNode_;
    std::vector<std::shared_ptr<Spawner>> spawners_;
    UniverseParameters params_;

    uint64_t tickIndex_ = 0;

    Tril::AutoClearingContainer<std::function<void(uint64_t tick)>> perTickTasks_;

    double GetLunarCycle() const;
};

#endif // UNIVERSE_H
