#include "SenseLunarCycle.h"

SenseLunarCycle::SenseLunarCycle(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner)
    : Sense(network, outputConnections, owner)
{
}

SenseLunarCycle::~SenseLunarCycle()
{
}

std::string SenseLunarCycle::GetDescription() const
{
    return "<p>The lunar cycle is a steadily fluctuating value that is updated,"
           " each tick. This value will be the same for all trilobytes with this "
           "sense, meaning that it could be possible for multiple trilobytes to "
           "coordinate behaviour by all reacting to a specific moon phase.</p>";
}

void SenseLunarCycle::PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& /*entities*/, const UniverseParameters& universeParameters) const
{
    inputs.at(0) = universeParameters.lunarCycle_;
}
