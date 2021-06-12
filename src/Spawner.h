#ifndef SPAWNER_H
#define SPAWNER_H

#include "DrawSettings.h"
#include "EntityContainerInterface.h"

#include <Energy.h>
#include <Shape.h>

#include <memory>

class QPainter;
class UniverseParameters;

class Spawner : public std::enable_shared_from_this<Spawner> {
public:
    enum class Shape {
        Circle,
        Square,
    };

    enum class Spawn {
        FoodPellet,
        MeatChunk,
        Spike,
    };

    Spawner(EntityContainerInterface& entityContainer, double x, double y, double radius, unsigned maxEntities, double ticksBetweenEntities, Shape shape, Spawn spawn);

    static void Draw(QPainter& paint, Spawn spawn, Shape shape, double x, double y, double radius, bool dashedEdge);

    double GetX() const { return x_; }
    double GetY() const { return y_; }
    double GetRadius() const { return radius_; }
    unsigned GetMaxEntities() const { return maxEntities_; }
    bool Contains(const Point& point) const;

    void Draw(QPainter& paint, const DrawSettings& options);
    void Tick(const UniverseParameters& params);
    void EntityRemoved();
    void AddEntitiesImmediately(unsigned entityCount);

private:
    constexpr static Energy entityEnergyContent_ = 30_mj;

    EntityContainerInterface& entityContainer_;

    double x_;
    double y_;
    double radius_;
    Shape shape_;
    Spawn spawn_;

    unsigned maxEntities_;
    double maxTicksTillNext_;
    double ticksTillNext_;

    unsigned currentEntityCount_;

    Circle GetCircleCollide() const { return Circle{ x_, y_, radius_ }; };
    Rect GetSquareCollide() const { return Rect{ x_ - radius_, y_ - radius_, x_ + radius_, y_ + radius_ }; };
    double GetSpawnEntityRadius() const;

    void SpawnEntity();
};

#endif // SPAWNER_H
