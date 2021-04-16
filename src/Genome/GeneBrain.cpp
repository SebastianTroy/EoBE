#include "GeneBrain.h"

#include "Phenotype.h"
#include <Energy.h>

using namespace nlohmann;

std::shared_ptr<Gene> GeneBrain::Generate(unsigned brainWidth)
{
    return std::make_shared<GeneBrain>(Random::Number(1, 5), brainWidth);
}

GeneBrain::GeneBrain(unsigned layerCount, unsigned width)
    : GeneBrain(std::make_shared<NeuralNetwork>(layerCount, width, NeuralNetwork::InitialWeights::Random), Random::Number(0.0, 100.0))
{
}

GeneBrain::GeneBrain(const std::shared_ptr<NeuralNetwork>& brain, double dominance)
    : Gene(dominance)
    , brain_(brain)
{
    // Modify connections
    AddMutation(BASE_WEIGHT, [&]()
    {
        brain_ = brain_->WithMutatedConnections();
    });

    // Insert a new row
    AddMutation(0.01 * BASE_WEIGHT, [&]()
    {
        unsigned newRowIndex = Random::Number(size_t{ 0 }, brain_->GetLayerCount());
        brain_ = brain_->WithRowAdded(newRowIndex, NeuralNetwork::InitialWeights::PassThrough);
    });

    // Remove a row
    AddMutation(0.01 * BASE_WEIGHT, [&]()
    {
        // Don't allow mutation to remove final row
        if (brain_->GetLayerCount() > 1) {
            unsigned newRowIndex = Random::Number(size_t{ 0 }, brain_->GetLayerCount());
            brain_ = brain_->WithRowRemoved(newRowIndex);
        }
    });

    // MAYBE add mutations for adding/removing columns (would require modifications to all senses and effectors connections, and knowledge of which index was effected by the mutation)
}

json GeneBrain::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_BRAIN, NeuralNetwork::Serialise(brain_)},
    };
}

void GeneBrain::ExpressGene(Trilobyte& /*owner*/, Phenotype& target) const
{
    target.brain = brain_;
    target.baseMetabolism += (1 + brain_->GetLayerCount()) * brain_->GetLayerWidth() * 0.15_uj;
}

std::shared_ptr<Gene> GeneBrain::Copy() const
{
    return std::make_shared<GeneBrain>(brain_, GetDominance());
}
