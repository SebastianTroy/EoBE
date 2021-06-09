#include "Spawner.h"

#include "FoodPellet.h"
#include "MeatChunk.h"
#include "Spike.h"
#include "UniverseParameters.h"

#include <Random.h>

#include <QPainter>

#include <memory>

Spawner::Spawner(EntityContainerInterface& entityContainer, double x, double y, double radius, double EntityDensity, Shape shape, Spawn spawn)
    : entityContainer_(entityContainer)
    , x_(x)
    , y_(y)
    , radius_(radius)
    , shape_(shape)
    , spawn_(spawn)
    , maxEntities_(std::pow(Tril::Pi * radius_, 2.0) * EntityDensity)
    , ticksTillNext_(0)
    , currentEntityCount_(0)
{
}

bool Spawner::Contains(const Point& point) const
{
    switch (shape_) {
    case Shape::Circle :
        return ::Contains(GetCircleCollide(), point);
    case Shape::Square :
        return ::Contains(GetSquareCollide(), point);
    }
    return false;
}

void Spawner::Draw(QPainter& paint, const DrawSettings& options)
{
    if (options.showFoodSpawners_) {
        paint.save();
        paint.setRenderHint(QPainter::RenderHint::Antialiasing, true);
        switch (spawn_) {
        case Spawn::FoodPellet :
            paint.setPen(QColor(0, 205, 90, 255));
            paint.setBrush(QColor(0, 205, 90, 190).lighter());
            break;
        case Spawn::MeatChunk :
            paint.setPen(QColor(184, 68, 68, 255));
            paint.setBrush(QColor(184, 68, 68, 190).lighter());
            break;
        case Spawn::Spike :
            paint.setPen(QColor(225, 225, 225, 255));
            paint.setBrush(QColor(225, 225, 225, 190).lighter());
            break;
        }

        QPen pen = paint.pen();
        pen.setCosmetic(true);
        pen.setWidthF(2.5);
        paint.setPen(pen);

        switch (shape_) {
        case Shape::Circle :
            paint.drawEllipse({x_, y_}, GetRadius(), GetRadius());
            break;
        case Shape::Square:
            paint.drawRect(x_ - radius_, y_ - radius_, GetRadius() * 2, GetRadius() * 2);
            break;
        }

        paint.restore();
    }
}

void Spawner::Tick(const UniverseParameters& params)
{
    while (ticksTillNext_ <= 0 && currentEntityCount_ < maxEntities_) {
        SpawnEntity();
    }
    ticksTillNext_ -= (params.foodSpawnRateModifier * 1.0);
}

void Spawner::EntityRemoved()
{
    --currentEntityCount_;
}

void Spawner::AddEntitiesImmediately(unsigned EntityCount)
{
    for (unsigned i = 0; i < EntityCount; i++) {
        SpawnEntity();
    }
    ticksTillNext_ = 0;
}

double Spawner::GetSpawnEntityRadius() const
{
    switch (spawn_) {
    case Spawn::FoodPellet :
        return FoodPellet::GetPelletRadius(entityEnergyContent_);
    case Spawn::MeatChunk :
        return MeatChunk::GetMeatChunkRadius(entityEnergyContent_);
    case Spawn::Spike :
        return Spike::RADIUS;
    }
    return 1.0;
}

void Spawner::SpawnEntity()
{
    Point location = shape_ == Shape::Circle ? Random::PointIn(GetCircleCollide()) : Random::PointIn(GetSquareCollide());
    if (entityContainer_.CountEntities(Circle{ location.x, location.y, GetSpawnEntityRadius() }) == 0) {

        auto deleter = [spawner = shared_from_this()](auto* entity)
        {
            --spawner->currentEntityCount_;
            delete entity;
        };

        switch (spawn_) {
        case Spawn::FoodPellet :
            entityContainer_.AddEntity(std::shared_ptr<FoodPellet>(new FoodPellet(entityEnergyContent_, Transform{ location.x, location.y, Random::Bearing() }), deleter));
            break;
        case Spawn::MeatChunk :
            entityContainer_.AddEntity(std::shared_ptr<MeatChunk>(new MeatChunk(entityEnergyContent_, Transform{ location.x, location.y, Random::Bearing() }, 0.0), deleter));
            break;
        case Spawn::Spike :
            entityContainer_.AddEntity(std::shared_ptr<Spike>(new Spike(Transform{ location.x, location.y, Random::Bearing() }), deleter));
            break;
        }
        ++currentEntityCount_;
        auto proportion = double(currentEntityCount_) / double(maxEntities_);
        ticksTillNext_ += 10.0 * ((-0.8 * (std::pow(proportion, 2.0) * -std::pow(proportion - 2, 2.0))) + 0.1);
    }
}
