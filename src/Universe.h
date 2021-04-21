#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "DrawSettings.h"
#include "FeedDispenser.h"
#include "LineGraph.h"
#include "QuadTree.h"

#include <Energy.h>
#include <AutoClearingContainer.h>

#include <QTimer>
#include <QPainter>

#include <iomanip>
#include <functional>
#include <math.h>

class Universe {
public:
    Universe(Rect startingQuad);

    void SetEntityTargetPerQuad(uint64_t target, uint64_t leeway);

    void AddEntity(const std::shared_ptr<Entity>& entity) { rootNode_.AddEntity(entity); }
    std::shared_ptr<Entity> PickEntity(const Point& location, bool remove) { return rootNode_.PickEntity(location, remove); }
    void ClearAllEntities() { rootNode_.Clear(); }
    template <typename... T>
    void ClearAllEntitiesOfType() { rootNode_.Clear<T...>(); }
    void ForEach(const std::function<void (const std::shared_ptr<Entity>& e)>& action) const { rootNode_.ForEach(action); }

    void AddFeedDispenser(const std::shared_ptr<FeedDispenser>& feeder) { feedDispensers_.push_back(feeder); }
    std::shared_ptr<FeedDispenser> PickFeedDispenser(const Point& location, bool remove);
    void ClearAllFeedDispensers() { feedDispensers_.clear(); }

    UniverseParameters& GetParameters() { return params_; }
    const EntityContainerInterface& GetEntityContainer() const { return rootNode_; }

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
    void Draw(QPainter& painter, const DrawSettings& options, const Rect& drawArea) const;

private:
    QuadTree rootNode_;
    std::vector<std::shared_ptr<FeedDispenser>> feedDispensers_;
    UniverseParameters params_;

    uint64_t tickIndex_ = 0;

    Tril::AutoClearingContainer<std::function<void(uint64_t tick)>> perTickTasks_;

    double GetLunarCycle() const;
};

#endif // UNIVERSE_H
