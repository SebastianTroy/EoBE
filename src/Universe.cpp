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
    : rootNode_(startingQuad)
{
    // TODO get rid of this default nonsense here
    feedDispensers_.push_back(std::make_shared<FeedDispenser>(rootNode_,  1000, 0, 950, 0.001));
    feedDispensers_.push_back(std::make_shared<FeedDispenser>(rootNode_, -1000, 0, 950, 0.001));

    for (auto& feeder : feedDispensers_) {
        feeder->AddPelletsImmediately(feeder->GetMaxPellets() / 8);
    }

    for (unsigned i = 0; i < 75u; ++i) {
        Point spikeLocation = Random::PointIn(Circle{ feedDispensers_.front()->GetX(), feedDispensers_.front()->GetY(), feedDispensers_.front()->GetRadius() });
        rootNode_.AddEntity(std::make_shared<Spike>(Transform{ spikeLocation.x, spikeLocation.y, Random::Bearing() }));
    }

    rootNode_.SetEntityTargetPerQuad(25, 5);

    for (const auto& feeder : feedDispensers_) {
        for (unsigned i = 0; i < std::max(size_t{ 1 }, 25 / feedDispensers_.size()); i++) {
            double rotation = Random::Number(0.0, Tril::Tau);
            double distance = std::sqrt(Random::Number(0.0, 1.0)) * feeder->GetRadius();
            double trilobyteX = feeder->GetX() + distance * std::cos(rotation);
            double trilobyteY = feeder->GetY() + distance * std::sin(rotation);
            rootNode_.AddEntity(std::make_shared<Trilobyte>(300_mj, Transform{ trilobyteX, trilobyteY, Random::Bearing() }, GeneFactory::Get().GenerateDefaultGenome(NeuralNetwork::BRAIN_WIDTH)));
        }
    }
}

void Universe::SetEntityTargetPerQuad(uint64_t target, uint64_t leeway)
{
    rootNode_.SetEntityTargetPerQuad(target, leeway);
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

void Universe::Draw(QPainter& p, const DrawSettings& options, const Rect& drawArea) const
{
    for (auto& dispenser : feedDispensers_) {
        dispenser->Draw(p, options);
    }
    rootNode_.Draw(p, options, drawArea);
}

void Universe::Tick()
{
    params_.lunarCycle_ = GetLunarCycle();

    rootNode_.Tick(params_);

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
