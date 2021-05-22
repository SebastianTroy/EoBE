#ifndef EFFECTORPROBOSCISMOUTH_H
#define EFFECTORPROBOSCISMOUTH_H

#include "Effector.h"
#include <Shape.h>

class EffectorProboscisMouth : public Effector {
public:
    EffectorProboscisMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner, const double& proboscisLength);
    ~EffectorProboscisMouth();

    virtual std::string_view GetName() const override { return "Proboscis"; }
    virtual std::string GetDescription() const override;
    virtual void Draw(QPainter& paint) const override;

private:
    double proboscisLength_;

    virtual Energy PerformActions(const std::vector<double>& actionValues, EntityContainerInterface& entities, const UniverseParameters& universeParameters) override;

    Line GetProboscis() const;
};

#endif // EFFECTORPROBOSCISMOUTH_H
