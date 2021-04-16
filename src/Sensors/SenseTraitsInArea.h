#ifndef SENSEENTITIESINAREA_H
#define SENSEENTITIESINAREA_H

#include "SenseTraitsBase.h"
#include <Shape.h>

class SenseTraitsInArea final : public SenseTraitsBase {
public:
    SenseTraitsInArea(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, double radius, const Transform& transform, std::vector<TraitNormaliser>&& toDetect);
    virtual ~SenseTraitsInArea() override {}

    virtual std::string_view GetName() const override { return "TraitsInArea"; }
    virtual std::string GetDescription() const override;

    virtual void Draw(QPainter& paint) const override;

private:
    double senseRadius_;
    Circle GetArea() const;

    virtual void FilterEntities(const EntityContainerInterface& entities, const std::function<void (const Entity&)>& forEachEntity) const override;
};

#endif // SENSEENTITIESINAREA_H
