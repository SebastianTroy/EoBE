#ifndef SENSEENTITIESTOUCHING_H
#define SENSEENTITIESTOUCHING_H

#include "SenseTraitsBase.h"
#include <Shape.h>

class QPainter;

class SenseTraitsTouching final : public SenseTraitsBase {
public:
    SenseTraitsTouching(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, Transform transform, std::vector<TraitNormaliser>&& toDetect);
    virtual ~SenseTraitsTouching() override {}

    virtual std::string_view GetName() const override { return "TraitsTouching"; }
    virtual std::string GetDescription() const override;

    virtual void Draw(QPainter& paint) const override;

private:
    Point GetPoint() const;

    virtual void FilterEntities(const EntityContainerInterface& entities, const std::function<void (const Entity&)>& forEachEntity) const override;
};

#endif // SENSEENTITIESTOUCHING_H
