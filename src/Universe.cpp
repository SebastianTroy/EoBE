#include "Universe.h"

#include "Trilobyte.h"
#include "FoodPellet.h"
#include "Egg.h"
#include "Spike.h"
#include "MainWindow.h"
#include "Genome/GeneFactory.h"
#include <Random.h>

#include <QVariant>

Universe::Universe(Rect startingQuad)
    : rootNode_(startingQuad, 25, 5, Entity::MAX_RADIUS * 2)
{
    // TODO get rid of this default nonsense here
    spawners_.push_back(std::make_shared<Spawner>(*this,  1000, -1000, 900, 50, 1000, Spawner::Shape::Square, Spawner::Spawn::Spike));
    spawners_.push_back(std::make_shared<Spawner>(*this,  1000, -1000, 950, 4500, 1.15, Spawner::Shape::Circle, Spawner::Spawn::FoodPellet));
    spawners_.push_back(std::make_shared<Spawner>(*this, -1000, -1000, 950, 4500, 1.15, Spawner::Shape::Circle, Spawner::Spawn::FoodPellet));
    spawners_.push_back(std::make_shared<Spawner>(*this,  1000,  1000, 900, 50, 1000, Spawner::Shape::Square, Spawner::Spawn::Spike));
    spawners_.push_back(std::make_shared<Spawner>(*this,  1000,  1000, 950, 2000, 0.15, Spawner::Shape::Circle, Spawner::Spawn::MeatChunk));
    spawners_.push_back(std::make_shared<Spawner>(*this, -1000,  1000, 950, 2000, 0.15, Spawner::Shape::Circle, Spawner::Spawn::MeatChunk));

    for (auto& spawner : spawners_) {
        spawner->AddEntitiesImmediately(spawner->GetMaxEntities() / 2.0);
    }

    rootNode_.SetItemCountTaregt(25);
    rootNode_.SetItemCountLeeway(5);

    for (const auto& spawner : spawners_) {
        for (unsigned i = 0; i < std::max(size_t{ 1 }, 25 / spawners_.size()); i++) {
            double rotation = Random::Number(0.0, Tril::Tau);
            double distance = std::sqrt(Random::Number(0.0, 1.0)) * spawner->GetRadius();
            double trilobyteX = spawner->GetX() + distance * std::cos(rotation);
            double trilobyteY = spawner->GetY() + distance * std::sin(rotation);
            rootNode_.Insert(std::make_shared<Trilobyte>(300_mj, Transform{ trilobyteX, trilobyteY, Random::Bearing() }, GeneFactory::Get().GenerateDefaultGenome(NeuralNetwork::BRAIN_WIDTH)));
        }
    }
}

void Universe::SetEntityTargetPerQuad(uint64_t target, uint64_t leeway)
{
    rootNode_.SetItemCountTaregt(target);
    rootNode_.SetItemCountLeeway(leeway);
}



void Universe::ForEachCollidingWith(const Point& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action)
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::QuadTreeIterator<Entity>([&](std::shared_ptr<Entity> item)
    {
        TRACE_LAMBDA("Entity->Action")
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Line& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action)
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::QuadTreeIterator<Entity>([&](std::shared_ptr<Entity> item)
    {
        TRACE_LAMBDA("Entity->Action")
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Rect& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action)
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::QuadTreeIterator<Entity>([&](std::shared_ptr<Entity> item)
    {
        TRACE_LAMBDA("Entity->Action")
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Circle& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action)
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::QuadTreeIterator<Entity>([&](std::shared_ptr<Entity> item)
    {
        TRACE_LAMBDA("Entity->Action")
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Point& collide, const std::function<void (const Entity&)>& action) const
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator<Entity>([&](const Entity& item)
    {
        TRACE_LAMBDA("Entity.Action")
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Line& collide, const std::function<void (const Entity&)>& action) const
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator<Entity>([&](const Entity& item)
    {
        TRACE_LAMBDA("Entity.Action")
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Rect& collide, const std::function<void (const Entity&)>& action) const
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator<Entity>([&](const Entity& item)
    {
        TRACE_LAMBDA("Entity.Action")
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Circle& collide, const std::function<void (const Entity&)>& action) const
{
    TRACE_FUNC()
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator<Entity>([&](const Entity& item)
    {
        TRACE_LAMBDA("Entity.Action")
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

std::shared_ptr<Entity> Universe::PickEntity(const Point& location, bool remove)
{
    TRACE_FUNC()
    std::shared_ptr<Entity> picked;
    rootNode_.ForEachItem(Tril::QuadTreeIterator<Entity>([&](std::shared_ptr<Entity> entity)
    {
        TRACE_LAMBDA("PickEntity")
        if (!picked) {
            picked = entity;
        }
    }).SetQuadFilter(BoundingRect(location, Entity::MAX_RADIUS)).SetItemFilter(location).SetRemoveItemPredicate([&](const Entity& entity)
    {
        return remove && picked && picked.get() == &entity;
    }));
    return picked;
}

std::shared_ptr<Spawner> Universe::PickSpawner(const Point& location, bool remove)
{
    TRACE_FUNC()
    auto iter = std::find_if(std::begin(spawners_), std::end(spawners_), [&](const auto& spawner)
    {
        TRACE_LAMBDA("PickSpawner")
        return spawner->Contains(location);
    });

    if (iter != spawners_.end()) {
        std::shared_ptr<Spawner> spawner = *iter;
        if (remove) {
            spawners_.erase(iter);
        }
        return spawner;
    }
    return nullptr;
}

Tril::Handle Universe::AddTask(std::function<void (uint64_t tick)>&& task)
{
    TRACE_FUNC()
    return perTickTasks_.PushBack(std::move(task));
}

void Universe::Draw(QPainter& p, const DrawSettings& options, const Rect& drawArea)
{
    TRACE_FUNC()
    for (auto& spawner : spawners_) {
        spawner->Draw(p, options);
    }
    if (options.showQuadTreeGrid_) {
        p.save();
        QPen pen(Qt::black);
        pen.setCosmetic(true);
        p.setPen(pen);
        rootNode_.ForEachQuad([&](const Rect& quadArea)
        {
            TRACE_FUNC()
            p.drawRect(QRectF(quadArea.left, quadArea.top, quadArea.right - quadArea.left, quadArea.bottom - quadArea.top));
        });
        p.restore();
    }
    rootNode_.ForEachItemNoRebalance(Tril::QuadTreeIterator<Entity>([&](std::shared_ptr<Entity> entity)
    {
        TRACE_LAMBDA("EntityDraw")
        entity->Draw(p, options);
    }).SetQuadFilter(BoundingRect(drawArea, Entity::MAX_RADIUS)));
}

std::vector<Property> Universe::GetProperties() const
{
    return {
        Property{
            "Ticks",
            [&]() -> std::string
            {
                return std::to_string(tickIndex_);
            },
            "The number of steps calculated by the simulation. A tick is the "
            "smallest measure of time in the simulation. Each tick, every Entity"
            " (Trilobyte, FoodPellet, Spike...) is processed, allowing them to "
            "move, eat, collide, etc.",
        },
        Property{
            "Entities",
            [&]() -> std::string
            {
                return std::to_string(rootNode_.Size());
            },
            "The total number of Entities that currently exist within the "
            "simulation. Individual entities can be selected for further "
            "information on them, it will appear as additional tabs in this "
            "view.",
        },
        Property{
            "Lunar Cycle",
            [&]() -> std::string
            {
                return std::to_string(params_.lunarCycle_);
            },
            "A value that oscillates at a fixed, regular interval. This value "
            "can be detected by Trilobytes giving them a shared external value "
            "that they can use to synchronise behaviour",
        },
    };
}

void Universe::Tick()
{
    TRACE_FUNC()
    params_.lunarCycle_ = GetLunarCycle();

    rootNode_.ForEachItem(Tril::QuadTreeIterator<Entity>([&](std::shared_ptr<Entity> entity)
    {
        TRACE_LAMBDA("EntityTick")
        entity->Tick(*this, params_);
    }).SetRemoveItemPredicate([](const Entity& entity)
    {
        return !entity.Exists();
    }));

    for (auto& spawner : spawners_) {
        spawner->Tick(params_);
    }

    perTickTasks_.ForEach([=](auto& task) -> void
    {
        std::invoke(task, tickIndex_);
    });

    ++tickIndex_;
}

double Universe::GetLunarCycle() const
{
    TRACE_FUNC()
    double shortCycle = 32; // ~1 full->new->full cycle 150 ticks
    double longCycle = 20; // 1 spring->neap->spring cycle per n short cycles
    return std::sin(tickIndex_ / shortCycle) * (0.75 - (0.25 * (std::sin(tickIndex_ / (shortCycle * longCycle)))));
}
