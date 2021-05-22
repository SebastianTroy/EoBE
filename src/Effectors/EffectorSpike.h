#ifndef EFFECTORSPIKE_H
#define EFFECTORSPIKE_H

#include "Effector.h"
#include <Shape.h>

class EffectorSpike : public Effector {
public:
    EffectorSpike(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner, double bearing, double length);
    ~EffectorSpike() override;

    virtual std::string_view GetName() const override { return "Spike"; }
    virtual std::string GetDescription() const override;

    virtual void Draw(QPainter& paint) const override;

private:
    double bearing_;
    double length_;

    virtual Energy PerformActions(const std::vector<double>& actionValues, EntityContainerInterface& entities, const UniverseParameters& universeParameters) override;

    Point GetTipOfSpike() const;
};

#endif // EFFECTORSPIKE_H
