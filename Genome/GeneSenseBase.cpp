#include "GeneSenseBase.h"


GeneSenseBase::GeneSenseBase(unsigned hiddenLayers, unsigned inputCount, unsigned outputCount)
    : GeneSenseBase(std::make_shared<NeuralNetwork>(hiddenLayers, inputCount, NeuralNetwork::InitialWeights::PassThrough), std::make_shared<NeuralNetworkConnector>(inputCount, outputCount), Random::Number(0.0, 100.0), Random::Number(0.0, 1.0))
{
}

GeneSenseBase::GeneSenseBase(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, double dominance, double mutationProbability)
    : Gene(dominance, mutationProbability)
    , network_(network)
    , outputConnections_(outputConnections)
{
    // Insert a new row
    AddMutation(0.15 * BASE_WEIGHT, [&]()
    {
        unsigned newRowIndex = Random::Number(0u, network_->GetLayerCount());
        network_ = network_->WithRowAdded(newRowIndex, NeuralNetwork::InitialWeights::PassThrough);
    });

    // Remove a row
    AddMutation(0.15 * BASE_WEIGHT, [&]()
    {
        // Don't allow mutation to remove final row
        if (network_->GetLayerCount() > 1) {
            unsigned newRowIndex = Random::Number(0u, network_->GetLayerCount());
            network_ = network_->WithRowRemoved(newRowIndex);
        }
    });

    // Modify internal network connections
    AddMutation(BASE_WEIGHT, [&]()
    {
        network_ = network_->WithMutatedConnections();
    });

    // Modify output connections
    AddMutation(BASE_WEIGHT, [&]()
    {
        outputConnections_ = outputConnections_->WithMutatedConnections();
    });
}

void GeneSenseBase::AddColumnInsertedMutation(double mutationWeight, std::function<void(unsigned index)>&& onColumnAdded)
{
    // Insert a new column
    AddMutation(mutationWeight, [&, onColumnAdded = std::move(onColumnAdded)]()
    {
        unsigned newColIndex = Random::Number(0u, network_->GetInputCount());
        network_ = network_->WithColumnAdded(newColIndex, NeuralNetwork::InitialWeights::Random);
        outputConnections_ = outputConnections_->WithInputAdded(newColIndex);
        onColumnAdded(newColIndex);
    });
}

void GeneSenseBase::AddColumnRemovedMutation(double mutationWeight, std::function<void(unsigned index)>&& onColumnRemoved)
{
    // Remove a column
    AddMutation(mutationWeight, [&, onColumnRemoved = std::move(onColumnRemoved)]()
    {
        // Don't allow mutation to remove final column
        if (network_->GetInputCount() > 1) {
            unsigned index = Random::Number(0u, network_->GetInputCount() - 1);
            network_ = network_->WithColumnRemoved(index);
            outputConnections_ = outputConnections_->WithInputRemoved(index);
            onColumnRemoved(index);
        }
    });
}