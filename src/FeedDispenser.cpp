#include "FeedDispenser.h"

#include "FoodPellet.h"
#include "UniverseParameters.h"

#include <Random.h>

#include <QPainter>

#include <memory>

FeedDispenser::FeedDispenser(EntityContainerInterface& entityContainer, double x, double y, double radius, double pelletDensity)
    : entityContainer_(entityContainer)
    , x_(x)
    , y_(y)
    , radius_(radius)
    , maxPellets_(std::pow(Tril::Pi * radius_, 2.0) * pelletDensity)
    , ticksTillNext_(0)
    , currentPelletCount_(0)
{
}

void FeedDispenser::Draw(QPainter& paint, const DrawSettings& options)
{
    if (options.showFoodSpawners_) {
        paint.save();
        paint.setPen(QColor(0, 205, 90, 255));
        paint.setBrush(QColor(0, 205, 90, 190).lighter());
        paint.drawEllipse({x_, y_}, GetRadius(), GetRadius());
        paint.restore();
    }
}

void FeedDispenser::Tick(const UniverseParameters& params)
{
    while (ticksTillNext_ <= 0 && currentPelletCount_ < maxPellets_) {
        SpawnPellet();
    }
    ticksTillNext_ -= (params.foodSpawnRateModifier * 1.0);
}

void FeedDispenser::PelletEaten()
{
    --currentPelletCount_;
}

void FeedDispenser::AddPelletsImmediately(unsigned pelletCount)
{
    for (unsigned i = 0; i < pelletCount; i++) {
        SpawnPellet();
    }
    ticksTillNext_ = 0;
}

void FeedDispenser::SpawnPellet()
{
    Point foodLocation = Random::PointIn(Circle{ x_, y_, radius_ });
    if (entityContainer_.CountEntities(Circle{ foodLocation.x, foodLocation.y, FoodPellet::GetPelletRadius(pelletEnergyContent_) }) == 0) {

        auto deleter = [dispenser = shared_from_this()](FoodPellet* pellet)
        {
            --dispenser->currentPelletCount_;
            delete pellet;
        };

        entityContainer_.AddEntity(std::shared_ptr<FoodPellet>(new FoodPellet(pelletEnergyContent_, Transform{ foodLocation.x, foodLocation.y, Random::Bearing() }), deleter));
        ++currentPelletCount_;
        auto proportion = double(currentPelletCount_) / double(maxPellets_);
        ticksTillNext_ += 10.0 * ((-0.8 * (std::pow(proportion, 2.0) * -std::pow(proportion - 2, 2.0))) + 0.1);
    }
}
