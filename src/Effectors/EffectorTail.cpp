#include "EffectorTail.h"

#include "Trilobyte.h"

#include <QPainter>

EffectorTail::EffectorTail(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner)
    : Effector(network, inputConnections, owner)
{
}

EffectorTail::~EffectorTail()
{
}

std::string EffectorTail::GetDescription() const
{
    // TODO prevent backwards swimming with tail, or at least make it slower for more energy
    return "<p>The tail allows a trilobyte to accelerate either forwards or "
           "backwards, and turn either left or right.</p>"
           "<p>Outputs:<ol>"
             "<li>Forward(+ve)/Backward(-ve) acceleration.</li>"
             "<li>Clockwise(+ve)/Anti-clockwise(-ve) turn.</li>"
             "<li>Anti-clockwise(+ve)/Clockwise(-ve) turn.</li>"
           "</ol></p>";
}

void EffectorTail::Draw(QPainter& /*paint*/) const
{
    // TODO
}

Energy EffectorTail::PerformActions(const std::vector<double>& actionValues, const EntityContainerInterface& /*entities*/, const UniverseParameters& /*universeParameters*/)
{
    // TODO allow evolvable range conversions for each of these
    const double& acceleration = actionValues.at(0);
    const double& clockwiseTorque = actionValues.at(1);
    const double& antiClockwiseTorque = actionValues.at(2);

    owner_.AdjustVelocity(acceleration);
    owner_.AdjustBearing(clockwiseTorque / Tril::Tau);
    owner_.AdjustBearing(antiClockwiseTorque / Tril::Tau);

    /*
     * We want to make acceleration costlier as an organism gets faster
     */
    Energy accelerationCost = std::abs(acceleration) * 75_uj;
    Energy clockwiseRotationCost = std::abs(clockwiseTorque) * 60_uj;
    Energy antiClockwiseRotationCost = std::abs(antiClockwiseTorque) * 60_uj;
    return accelerationCost + clockwiseRotationCost + antiClockwiseRotationCost;
}
