#include "GeneEffectorFilterMouth.h"

#include "Trilobyte.h"
#include "Phenotype.h"
#include "Effectors/EffectorFilterMouth.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneEffectorFilterMouth::Generate(unsigned brainWidth)
{
    unsigned hiddenLayers = Random::Number(0u, 1u);
    return std::make_shared<GeneEffectorFilterMouth>(hiddenLayers, brainWidth);
}

GeneEffectorFilterMouth::GeneEffectorFilterMouth(unsigned /*hiddenLayers*/, unsigned inputCount)
    : GeneEffectorBase(0, inputCount, 0)
    // : GeneEffectorBase(hiddenLayers, inputCount, 1)
{
}

GeneEffectorFilterMouth::GeneEffectorFilterMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance)
    : GeneEffectorBase(network, inputConnections, dominance)
{
}

GeneEffectorFilterMouth::~GeneEffectorFilterMouth()
{
}

json GeneEffectorFilterMouth::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork())},
        {KEY_INPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetInputConnections())},
    };
}

void GeneEffectorFilterMouth::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.effectors.push_back(std::make_shared<EffectorFilterMouth>(GetNetwork(), GetInputConnections(), owner));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneEffectorFilterMouth::Copy() const
{
    return std::make_shared<GeneEffectorFilterMouth>(GetNetwork(), GetInputConnections(), GetDominance());
}
