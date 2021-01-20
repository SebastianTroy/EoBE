#include "Universe.h"

#include "Swimmer.h"
#include "FeedDispenser.h"
#include "FoodPellet.h"
#include "Egg.h"
#include "Spike.h"
#include "Random.h"
#include "MainWindow.h"
#include "Genome/GeneFactory.h"

#include <QVariant>

// TODO RenderSettings struct to contain such as showQuads, showSenseRanges, showPole, showFoodSpawnArea

Universe::Universe(UniverseObserver& focusInterface)
    : observerInterface_(focusInterface)
    , rootNode_({0, 0, 1000, 1000})
{
    Reset();

    /*
     * QT hack to get this running in the QT event loop (necessary for
     * drawing anything, not ideal for running the Sim quickly...)
     */
    mainThread_.setSingleShot(false);
    mainThread_.connect(&mainThread_, &QTimer::timeout, [&]() { Thread(); });
    mainThread_.start();
    UpdateTps();
}

void Universe::SetLimitTickRate(bool limited)
{
    limitSim_ = limited;
    UpdateTps();
}

void Universe::SetTpsTarget(int tps)
{
    targetTps_ = tps;
    UpdateTps();
}

void Universe::SetEntityTargetPerQuad(uint64_t target, uint64_t leeway)
{
    rootNode_.SetEntityTargetPerQuad(target, leeway);
}

const std::shared_ptr<Entity>& Universe::SelectEntity(const Point& location)
{
    selectedEntity_.reset();
    rootNode_.ForEachCollidingWith(location, [&](const std::shared_ptr<Entity>& e)
    {
        selectedEntity_ = e;
    });
    return selectedEntity_;
}

void Universe::SelectFittestSwimmer() {
    unsigned mostLivingChildren = 0;
    rootNode_.ForEach([&](const std::shared_ptr<Entity>& e)
    {
        if (const auto* s = dynamic_cast<const Swimmer*>(e.get())) {
            if (s->GetLivingDescendantsCount(2) > mostLivingChildren) {
                mostLivingChildren = s->GetLivingDescendantsCount(2);
                selectedEntity_ = e;
            }
        }
    });
    observerInterface_.SuggestUpdate();
}

void Universe::AddDefaultSwimmer(double x, double y)
{
    rootNode_.AddEntity(std::make_shared<Swimmer>(300_mj, Transform{ x, y, Random::Bearing() }, GeneFactory::DefaultGenome()));
}

void Universe::AddRandomSwimmer(double x, double y)
{
    rootNode_.AddEntity(std::make_shared<Swimmer>(300_mj, Transform{ x, y, Random::Bearing() }, GeneFactory::RandomGenome()));
}

EoBE::Handle Universe::AddTask(std::function<void (uint64_t tick)>&& task)
{
    return perTickTasks_.PushBack(std::move(task));
}

void Universe::Render(QPainter& p) const
{
    if (spawnFood_) {
        for (auto& dispenser : feedDispensers_) {
            dispenser->Draw(p);
        }
    }
    rootNode_.Draw(p);

    // TODO remove all this once it is in a GUI
    p.save();
    p.resetMatrix();
    p.setPen(Qt::black);
    p.setBackground(QColor(200, 255, 255));
    p.setBackgroundMode(Qt::BGMode::OpaqueMode);
    int line = 0;
    if (selectedEntity_ && dynamic_cast<Swimmer*>(selectedEntity_.get())) {
        auto f = dynamic_cast<Swimmer*>(selectedEntity_.get());
        p.drawText(0, line += 15, QString("   - %1 Ticks Old").arg(f->GetAge()));
        p.drawText(0, line += 15, QString("   - Laid %1 Eggs").arg(f->GetEggsLayedCount()));
        p.drawText(0, line += 15, QString("   - Children %1/%2").arg(f->GetLivingDescendantsCount(1)).arg(f->GetTotalDescendantsCount(1)));
        p.drawText(0, line += 15, QString("   - GrandChildred %1/%2").arg(f->GetLivingDescendantsCount(2)).arg(f->GetTotalDescendantsCount(2)));
        p.drawText(0, line += 15, QString("   - GreatGrandChildred %1/%2").arg(f->GetLivingDescendantsCount(3)).arg(f->GetTotalDescendantsCount(3)));
        p.drawText(0, line += 15, QString("   - All Descendants %1/%2").arg(f->GetLivingDescendantsCount()).arg(f->GetTotalDescendantsCount()));
        p.drawText(0, line += 15, QString("   - Energy %1mj").arg(f->GetEnergy() / 1_mj));
        p.drawText(0, line += 15, QString("   - Metabolism %1uj").arg(f->GetBaseMetabolism() / 1_uj));
        p.drawText(0, line += 15, QString("   - Health %1").arg(f->GetHealth()));
    }
    p.restore();
}

void Universe::ForEach(const std::function<void (const std::shared_ptr<Entity>&)>& action) const
{
    rootNode_.ForEach(action);
}

double Universe::GetLunarCycle() const
{
    double shortCycle = 32; // ~1 full->new->full cycle 150 ticks
    double longCycle = 20; // 1 spring->neap->spring cycle per n short cycles
    return std::sin(tickIndex_ / shortCycle) * (0.75 - (0.25 * (std::sin(tickIndex_ / (shortCycle * longCycle)))));
}

void Universe::Thread()
{
    if (reset_) {
        reset_ = false;

        feedDispensers_.clear();
        tickIndex_ = 0;
        rootNode_.Clear();
        selectedEntity_.reset();
        respawn_ = true;

        feedDispensers_.push_back(std::make_shared<FeedDispenser>(rootNode_,  1000, 0, 950, 0.001));
        feedDispensers_.push_back(std::make_shared<FeedDispenser>(rootNode_, -1000, 0, 950, 0.001));

        for (auto& feeder : feedDispensers_) {
            feeder->AddPelletsImmediately(feeder->GetMaxPellets() / 8);
        }

        for (unsigned i = 0; i < 75u; ++i) {
            Point spikeLocation = Random::PointInCircle({ feedDispensers_.front()->GetX(), feedDispensers_.front()->GetY(), feedDispensers_.front()->GetRadius() });
            rootNode_.AddEntity(std::make_shared<Spike>(Transform{ spikeLocation.x, spikeLocation.y, Random::Bearing() }));
        }

        rootNode_.SetEntityTargetPerQuad(25, 5);
        observerInterface_.SuggestUpdate();
    }

    if (removeAllSwimmers_) {
        removeAllSwimmers_ = false;
        rootNode_.Clear<Swimmer, Egg>();
    }

    if (removeAllFood_) {
        removeAllFood_ = false;
        rootNode_.Clear<FoodPellet>();
    }

    if (respawn_) {
        respawn_ = false;
        for (const auto& feeder : feedDispensers_) {
            for (unsigned i = 0; i < std::max(1u, 25 / feedDispensers_.size()); i++) {
                double rotation = Random::Number(0.0, EoBE::Tau);
                double distance = std::sqrt(Random::Number(0.0, 1.0)) * feeder->GetRadius();
                double swimmerX = feeder->GetX() + distance * std::cos(rotation);
                double swimmerY = feeder->GetY() + distance * std::sin(rotation);
                AddDefaultSwimmer(swimmerX, swimmerY);
            }
        }
        observerInterface_.SuggestUpdate();
    }

    if (!pauseSim_ && targetTps_ > 0) {
        Tick();
    }
}

void Universe::Tick()
{
    universeParameters_.lunarCycle_ = GetLunarCycle();

    observerInterface_.SuggestUpdate();
    rootNode_.Tick(universeParameters_);

    if (autoSelectFittest_) {
        SelectFittestSwimmer();
    }

    if (spawnFood_) {
        for (auto& dispenser : feedDispensers_) {
            dispenser->Tick();
        }
    }

    perTickTasks_.ForEach([=](auto& task) -> void
    {
        std::invoke(task, tickIndex_);
    });

    ++tickIndex_;
}

void Universe::UpdateTps()
{
    mainThread_.setInterval(limitSim_ ? (1000 / std::max(targetTps_, 1u)) : 0);
}
