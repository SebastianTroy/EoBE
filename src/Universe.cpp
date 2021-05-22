#include "Universe.h"

#include "Trilobyte.h"
#include "FeedDispenser.h"
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
    feedDispensers_.push_back(std::make_shared<FeedDispenser>(*this,  1000, 0, 950, 0.001));
    feedDispensers_.push_back(std::make_shared<FeedDispenser>(*this, -1000, 0, 950, 0.001));

    for (auto& feeder : feedDispensers_) {
        feeder->AddPelletsImmediately(feeder->GetMaxPellets() / 8);
    }

    for (unsigned i = 0; i < 75u; ++i) {
        Point spikeLocation = Random::PointIn(Circle{ feedDispensers_.front()->GetX(), feedDispensers_.front()->GetY(), feedDispensers_.front()->GetRadius() });
        rootNode_.Insert(std::make_shared<Spike>(Transform{ spikeLocation.x, spikeLocation.y, Random::Bearing() }));
    }

    rootNode_.SetItemCountTaregt(25);
    rootNode_.SetItemCountLeeway(5);

    for (const auto& feeder : feedDispensers_) {
        for (unsigned i = 0; i < std::max(size_t{ 1 }, 25 / feedDispensers_.size()); i++) {
            double rotation = Random::Number(0.0, Tril::Tau);
            double distance = std::sqrt(Random::Number(0.0, 1.0)) * feeder->GetRadius();
            double trilobyteX = feeder->GetX() + distance * std::cos(rotation);
            double trilobyteY = feeder->GetY() + distance * std::sin(rotation);
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
    rootNode_.ForEachItem(Tril::QuadTreeIterator::Create<Entity>([&](std::shared_ptr<Entity> item)
    {
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Line& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action)
{
    rootNode_.ForEachItem(Tril::QuadTreeIterator::Create<Entity>([&](std::shared_ptr<Entity> item)
    {
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Rect& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action)
{
    rootNode_.ForEachItem(Tril::QuadTreeIterator::Create<Entity>([&](std::shared_ptr<Entity> item)
    {
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Circle& collide, const std::function<void (const std::shared_ptr<Entity>&)>& action)
{
    rootNode_.ForEachItem(Tril::QuadTreeIterator::Create<Entity>([&](std::shared_ptr<Entity> item)
    {
        if (Collides(collide, item->GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Point& collide, const std::function<void (const Entity&)>& action) const
{
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator::Create<Entity>([&](const Entity& item)
    {
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Line& collide, const std::function<void (const Entity&)>& action) const
{
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator::Create<Entity>([&](const Entity& item)
    {
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Rect& collide, const std::function<void (const Entity&)>& action) const
{
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator::Create<Entity>([&](const Entity& item)
    {
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

void Universe::ForEachCollidingWith(const Circle& collide, const std::function<void (const Entity&)>& action) const
{
    rootNode_.ForEachItem(Tril::ConstQuadTreeIterator::Create<Entity>([&](const Entity& item)
    {
        if (Collides(collide, item.GetCollide())) {
            action(item);
        }
    }).SetQuadFilter(BoundingRect(collide, Entity::MAX_RADIUS)).SetItemFilter(collide));
}

std::shared_ptr<Entity> Universe::PickEntity(const Point& location, bool remove)
{
    std::shared_ptr<Entity> picked;
    rootNode_.ForEachItem(Tril::QuadTreeIterator::Create<Entity>([&](std::shared_ptr<Entity> entity)
    {
        if (!picked) {
            picked = entity;
        }
    }).SetQuadFilter(BoundingRect(location, Entity::MAX_RADIUS)).SetItemFilter(location).SetRemoveItemPredicate<Entity>([&](const Entity& entity)
    {
        return remove && picked && picked.get() == &entity;
    }));
    return picked;
}

std::shared_ptr<FeedDispenser> Universe::PickFeedDispenser(const Point& location, bool remove)
{
    auto iter = std::find_if(std::begin(feedDispensers_), std::end(feedDispensers_), [&](const auto& dispenser)
    {
        return Contains(dispenser->GetCollide(), location);
    });

    if (iter != feedDispensers_.end()) {
        if (remove) {
            feedDispensers_.erase(iter);
        }
        return *iter;
    }
    return nullptr;
}

Tril::Handle Universe::AddTask(std::function<void (uint64_t tick)>&& task)
{
    return perTickTasks_.PushBack(std::move(task));
}

void Universe::Draw(QPainter& p, const DrawSettings& options, const Rect& drawArea)
{
    for (auto& dispenser : feedDispensers_) {
        dispenser->Draw(p, options);
    }
    rootNode_.ForEachItem(Tril::QuadTreeIterator::Create<Entity>([&](std::shared_ptr<Entity> entity)
    {
        entity->Draw(p, options);
    }).SetQuadFilter(BoundingRect(drawArea, Entity::MAX_RADIUS)).SetItemFilter(BoundingRect(drawArea, Entity::MAX_RADIUS)));
}

void Universe::Tick()
{
    params_.lunarCycle_ = GetLunarCycle();

    rootNode_.ForEachItem(Tril::QuadTreeIterator::Create<Entity>([&](std::shared_ptr<Entity> entity)
    {
        entity->Tick(*this, params_);
    }).SetRemoveItemPredicate<Entity>([](const Entity& entity)
    {
        return !entity.Exists();
    }));

    for (auto& dispenser : feedDispensers_) {
        dispenser->Tick(params_);
    }

    perTickTasks_.ForEach([=](auto& task) -> void
    {
        std::invoke(task, tickIndex_);
    });

    ++tickIndex_;
}

double Universe::GetLunarCycle() const
{
    double shortCycle = 32; // ~1 full->new->full cycle 150 ticks
    double longCycle = 20; // 1 spring->neap->spring cycle per n short cycles
    return std::sin(tickIndex_ / shortCycle) * (0.75 - (0.25 * (std::sin(tickIndex_ / (shortCycle * longCycle)))));
}
