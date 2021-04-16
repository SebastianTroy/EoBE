#include "GeneEffectorProboscisMouth.h"

#include "Trilobyte.h"
#include "Phenotype.h"
#include "Effectors/EffectorProboscisMouth.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneEffectorProboscisMouth::Generate(unsigned brainWidth)
{
    return std::make_shared<GeneEffectorProboscisMouth>(brainWidth, Random::Gaussian(15.0, 5.0));
}

GeneEffectorProboscisMouth::GeneEffectorProboscisMouth(unsigned inputCount, double proboscisLength)
    : GeneEffectorBase(0, inputCount, 0)
    , proboscisLength_(proboscisLength)
{
    AddMutations();
}

GeneEffectorProboscisMouth::GeneEffectorProboscisMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance, double proboscisLength)
    : GeneEffectorBase(network, inputConnections, dominance)
    , proboscisLength_(proboscisLength)
{
    AddMutations();
}

GeneEffectorProboscisMouth::~GeneEffectorProboscisMouth()
{
}

json GeneEffectorProboscisMouth::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork())},
        {KEY_INPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetInputConnections())},
        {KEY_LENGTH, proboscisLength_},
    };
}

void GeneEffectorProboscisMouth::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.effectors.push_back(std::make_shared<EffectorProboscisMouth>(GetNetwork(), GetInputConnections(), owner, proboscisLength_));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneEffectorProboscisMouth::Copy() const
{
    return std::make_shared<GeneEffectorProboscisMouth>(GetNetwork(), GetInputConnections(), GetDominance(), proboscisLength_);
}

void GeneEffectorProboscisMouth::AddMutations()
{
    // Mutate the length on average 0 pixels
    AddMutation(BASE_WEIGHT, [&]()
    {
        proboscisLength_ += Random::Gaussian(0.0, 3.0);
        // Do not allow the proboscis to be less than 1.0
        proboscisLength_ = std::max(minProboscisLength_, proboscisLength_);
    });
}
