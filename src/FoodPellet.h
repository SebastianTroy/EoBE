#ifndef FOODPELLET_H
#define FOODPELLET_H

#include "Entity.h"

class FeedDispenser;

class QPainter;

class FoodPellet : public Entity {
public:
    static double GetPelletRadius(const Energy& energy);

    FoodPellet(const std::shared_ptr<FeedDispenser>& spawner, Energy energy, const Transform& transform);
    ~FoodPellet() override;

    virtual std::string_view GetName() const override { return "FoodPellet"; }
    virtual std::string_view GetDescription() const override;

private:
    const std::shared_ptr<FeedDispenser> spawner_;

    virtual void TickImpl(EntityContainerInterface& /*container*/, const UniverseParameters& /*universeParameters*/) override final {}
    virtual void DrawExtras(QPainter& paint) override final;

private:
    virtual std::vector<Property> CollectProperties() const override { return {}; }
};

#endif // FOODPELLET_H
