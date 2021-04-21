#ifndef FOODPELLET_H
#define FOODPELLET_H

#include "Entity.h"

class FeedDispenser;

class QPainter;

class FoodPellet : public Entity {
public:
    static double GetPelletRadius(const Energy& energy);

    FoodPellet(Energy energy, const Transform& transform);
    ~FoodPellet() override {}

    std::string_view GetName() const override { return "FoodPellet"; }
    std::string_view GetDescription() const override;

private:
    void TickImpl(EntityContainerInterface& /*container*/, const UniverseParameters& /*universeParameters*/) override final {}
};

#endif // FOODPELLET_H
