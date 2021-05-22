#include "Effector.h"

#include "Trilobyte.h"

Effector::Effector(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner)
    : owner_(owner)
    , network_(network)
    , inputConnections_(inputConnections)
    , outputs_(inputConnections->GetOutputCount(), 0.0)
{
}

Effector::~Effector()
{
}

Energy Effector::Tick(const std::vector<double>& inputs, EntityContainerInterface& entities, const UniverseParameters& universeParameters)
{
    std::fill(std::begin(outputs_), std::end(outputs_), 0.0);
    inputConnections_->PassForward(inputs, outputs_);
    network_->ForwardPropogate(outputs_);
    return PerformActions(outputs_, entities, universeParameters);
}
