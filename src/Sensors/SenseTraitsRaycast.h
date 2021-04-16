#ifndef SENSEENTITYRAYCAST_H
#define SENSEENTITYRAYCAST_H

#include "SenseTraitsBase.h"

class SenseTraitsRaycast final : public SenseTraitsBase {
public:
    SenseTraitsRaycast(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<TraitNormaliser>&& toDetect, const Transform& transform, double maxDistance, double angle);
    virtual ~SenseTraitsRaycast() override {}

    virtual std::string_view GetName() const override { return "TraitsRaycast"; }
    virtual std::string GetDescription() const override;

    virtual void Draw(QPainter& paint) const override;

private:
    double rayCastDistance_;
    double rayCastAngle_;
    Line GetLine() const;

    virtual void FilterEntities(const EntityContainerInterface& entities, const std::function<void (const Entity&)>& forEachEntity) const override;
};

#endif // SENSEENTITYRAYCAST_H
