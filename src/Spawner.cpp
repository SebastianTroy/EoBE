#include "Spawner.h"

#include "FoodPellet.h"
#include "MeatChunk.h"
#include "Spike.h"
#include "UniverseParameters.h"

#include <Random.h>

#include <QPainter>

#include <memory>

Spawner::Spawner(EntityContainerInterface& entityContainer, double x, double y, double radius, unsigned maxEntities, double ticksBetweenEntities, Shape shape, Spawn spawn)
    : entityContainer_(entityContainer)
    , x_(x)
    , y_(y)
    , radius_(radius)
    , shape_(shape)
    , spawn_(spawn)
    , maxEntities_(maxEntities)
    , maxTicksTillNext_(ticksBetweenEntities)
    , ticksTillNext_(0)
    , currentEntityCount_(0)
{
}

void Spawner::Draw(QPainter& paint, Spawn spawn, Shape shape, double x, double y, double radius, bool dashedEdge)
{
    paint.save();
    paint.setRenderHint(QPainter::RenderHint::Antialiasing, true);

    switch (spawn) {
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
    if (dashedEdge) {
        pen.setDashPattern({ 20.0, 10.0 });
    }
    paint.setPen(pen);

    switch (shape) {
    case Shape::Circle :
        paint.drawEllipse({x, y}, radius, radius);
        break;
    case Shape::Square:
        paint.drawRect(x - radius, y - radius, radius * 2, radius * 2);
        break;
    }

    paint.restore();
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
    if (options.showSpawners_) {
        Draw(paint, spawn_, shape_, x_, y_, radius_, false);
    }
}

void Spawner::Tick(const UniverseParameters& params)
{
    if (params.spawnRateModifier <= 0) {
        return;
    }

    while (currentEntityCount_ < maxEntities_ && ticksTillNext_ <= 0) {
        SpawnEntity();
        ticksTillNext_ += maxTicksTillNext_ / params.spawnRateModifier;
    }

    --ticksTillNext_;
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
    }
}
