#include "Effector.h"

#include "Swimmer.h"

Effector::Effector(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Swimmer& owner)
    : owner_(owner)
    , network_(network)
    , inputConnections_(inputConnections)
    , outputs_(inputConnections->GetOutputCount(), 0.0)
{
}

Effector::~Effector()
{
}

Energy Effector::Tick(const std::vector<double>& inputs, const EntityContainerInterface& entities)
{
    std::fill(std::begin(outputs_), std::end(outputs_), 0.0);
    inputConnections_->PassForward(inputs, outputs_);
    network_->ForwardPropogate(outputs_);
    return PerformActions(outputs_, entities);
}
