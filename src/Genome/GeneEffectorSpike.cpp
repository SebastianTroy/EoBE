#include "GeneEffectorSpike.h"

#include "Trilobyte.h"
#include "Effectors/EffectorSpike.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneEffectorSpike::Generate(unsigned brainWidth)
{
    double bearing = Random::Bearing();
    double length = Random::Number(3.5, 15.5);
    return std::make_shared<GeneEffectorSpike>(brainWidth, bearing, length);
}

GeneEffectorSpike::GeneEffectorSpike(unsigned inputCount, double bearing, double length)
    : GeneEffectorBase(0, inputCount, 0)
    , bearing_(bearing)
    , length_(length)
{
    AddMutations();
}

GeneEffectorSpike::GeneEffectorSpike(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance, double bearing, double length)
    : GeneEffectorBase(network, inputConnections, dominance)
    , bearing_(bearing)
    , length_(length)
{
    AddMutations();
}

GeneEffectorSpike::~GeneEffectorSpike()
{
}

json GeneEffectorSpike::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork())},
        {KEY_INPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetInputConnections())},
        {KEY_BEARING, bearing_},
        {KEY_LENGTH, length_},
    };
}

void GeneEffectorSpike::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.effectors.push_back(std::make_shared<EffectorSpike>(GetNetwork(), GetInputConnections(), owner, bearing_, length_));
    target.baseMetabolism += GetMetabolicCost();
    // TODO Spikes neeed to cost some energy to grow initially, perhaps use some energy here based on spike length?
}

std::shared_ptr<Gene> GeneEffectorSpike::Copy() const
{
    return std::make_shared<GeneEffectorSpike>(GetNetwork(), GetInputConnections(), GetDominance(), bearing_, length_);
}

void GeneEffectorSpike::AddMutations()
{
    // Adjust bearing
    AddMutation(BASE_WEIGHT, [&]()
    {
        bearing_ = std::abs(std::fmod(Random::Gaussian(bearing_, Tril::Tau / 25), Tril::Tau));
    });
    // Adjust length
    AddMutation(BASE_WEIGHT, [&]()
    {
        length_ = Random::Gaussian(length_, 0.5);
    });
}
