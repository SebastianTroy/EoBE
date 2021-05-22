#ifndef FEEDDISPENSER_H
#define FEEDDISPENSER_H

#include "DrawSettings.h"
#include "EntityContainerInterface.h"

#include <Energy.h>
#include <Shape.h>

#include <memory>

class QPainter;
class UniverseParameters;

class FeedDispenser : public std::enable_shared_from_this<FeedDispenser> {
public:
    FeedDispenser(EntityContainerInterface& entityContainer, double x, double y, double radius, double pelletDensity);

    double GetX() const { return x_; }
    double GetY() const { return y_; }
    double GetRadius() const { return radius_; }
    unsigned GetMaxPellets() const { return maxPellets_; }
    Circle GetCollide() const { return Circle{ x_, y_, radius_ }; };

    void Draw(QPainter& paint, const DrawSettings& options);
    void Tick(const UniverseParameters& params);
    void PelletEaten();
    void AddPelletsImmediately(unsigned pelletCount);

private:
    constexpr static Energy pelletEnergyContent_ = 30_mj;

    EntityContainerInterface& entityContainer_;

    double x_;
    double y_;
    double radius_;

    unsigned maxPellets_;
    double ticksTillNext_;

    unsigned currentPelletCount_;

    void SpawnPellet();
};

#endif // FEEDDISPENSER_H
