#include "Sense.h"

#include "Swimmer.h"
#include "EntityContainerInterface.h"

#include <QPainter>

Sense::Sense(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Swimmer& owner)
    : owner_(owner)
    , network_(network)
    , outputConnections_(outputConnections)
    , inputs_(outputConnections->GetInputCount(), 0.0)
{
}

Sense::~Sense()
{
}

void Sense::Draw(QPainter& /*paint*/) const
{
    // Nothing to draw
}

void Sense::Tick(std::vector<double>& outputs, const EntityContainerInterface& entities, const Sense::UniverseInfoStructRef& environment)
{
    PrimeInputs(inputs_, entities, environment);
    network_->ForwardPropogate(inputs_);
    outputConnections_->PassForward(inputs_, outputs);
}

