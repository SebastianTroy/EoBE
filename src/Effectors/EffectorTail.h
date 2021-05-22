#ifndef EFFECTORTAIL_H
#define EFFECTORTAIL_H

#include "Effector.h"

class EffectorTail : public Effector {
public:
    EffectorTail(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner);
    virtual ~EffectorTail() override;

    virtual std::string_view GetName() const override { return "Tail"; }
    virtual std::string GetDescription() const override;

    virtual void Draw(QPainter& paint) const override;

private:
    virtual Energy PerformActions(const std::vector<double>& actionValues, EntityContainerInterface& entities, const UniverseParameters& universeParameters) override;
};

#endif // EFFECTORTAIL_H
