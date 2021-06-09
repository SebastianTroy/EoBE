#ifndef SPIKE_H
#define SPIKE_H

#include "Entity.h"

class Spike : public Entity {
public:
    constexpr static double RADIUS = 12.0;

    Spike(const Transform& transform);
    ~Spike() override;

    virtual std::string_view GetName() const override { return "Spike"; }
    virtual std::string_view GetDescription() const override;

protected:
    virtual void TickImpl(EntityContainerInterface& container, const UniverseParameters& universeParameters) override;

private:
    virtual std::vector<Property> CollectProperties() const override;
};

#endif // SPIKE_H
