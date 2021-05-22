#ifndef EFFECTORSPRINGTAIL_H
#define EFFECTORSPRINGTAIL_H

#include <RangeConverter.h>

#include "Effector.h"

class EffectorSpringTail : public Effector {
public:
    EffectorSpringTail(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner, const Tril::RangeConverter& neuronExcitementToChargeRate, const Energy& storedEnergyCap, const double& triggerThreshold);
    ~EffectorSpringTail() override;

    virtual std::string_view GetName() const override { return "SpringTail"; }
    virtual std::string GetDescription() const override;

    virtual void Draw(QPainter& paint) const override;

private:
    const Tril::RangeConverter neuronExcitementToChargeRate_;
    const Energy storedEnergyCap_;
    const double triggerThreshold_;

    Energy storedEnergy_;

    virtual Energy PerformActions(const std::vector<double>& actionValues, EntityContainerInterface& entities, const UniverseParameters& universeParameters) override;
};

#endif // EFFECTORSPRINGTAIL_H
