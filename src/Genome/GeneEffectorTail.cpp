#include "GeneEffectorTail.h"

#include "Trilobyte.h"
#include "Effectors/EffectorTail.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneEffectorTail::Generate(unsigned brainWidth)
{
    unsigned hiddenLayers = Random::Number(0u, 2u);
    return std::make_shared<GeneEffectorTail>(hiddenLayers, brainWidth);
}

GeneEffectorTail::GeneEffectorTail(unsigned hiddenLayers, unsigned inputCount)
    : GeneEffectorBase(hiddenLayers, inputCount, 3)
{
    // No additional mutations
}

GeneEffectorTail::GeneEffectorTail(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance)
    : GeneEffectorBase(network, inputConnections, dominance)
{
    // No additional mutations
}

json GeneEffectorTail::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork())},
        {KEY_INPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetInputConnections())},
    };
}

void GeneEffectorTail::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.effectors.push_back(std::make_shared<EffectorTail>(GetNetwork(), GetInputConnections(), owner));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneEffectorTail::Copy() const
{
    return std::make_shared<GeneEffectorTail>(GetNetwork(), GetInputConnections(), GetDominance());
}
