#ifndef EFFECTORFILTERMOUTH_H
#define EFFECTORFILTERMOUTH_H

#include "Effector.h"

/**
 * The aim for this class is to allow trilobytes to eat food particals below a
 * critical size threshold. A single input allows the trilobyte to open and close
 * the mouth, making capture efficiency swing between 0% and 100%. The point of
 * closing the mouth will be a lower drag coefficient.
 */
class EffectorFilterMouth : public Effector {
public:
    EffectorFilterMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner);
    virtual ~EffectorFilterMouth() override;

    virtual std::string_view GetName() const override { return "FilterMouth"; }
    virtual std::string GetDescription() const override;

    virtual void Draw(QPainter& /*paint*/) const override { /* TODO once the mouth actually has a position/shape */ }

private:
    constexpr static double FOOD_RADIUS_THRESHOLD = 2.5;

    virtual Energy PerformActions(const std::vector<double>& actionValues, EntityContainerInterface& entities, const UniverseParameters& universeParameters) override;
};

#endif // EFFECTORFILTERMOUTH_H
