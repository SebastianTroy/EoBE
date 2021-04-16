#include "GeneSenseTraitsSelf.h"

#include "Phenotype.h"
#include "Sensors/SenseTraitsSelf.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneSenseTraitsSelf::Generate(unsigned brainWidth)
{
    std::vector<SenseTraitsBase::TraitNormaliser> traits;
    // Between 1 - 4 traits
    Random::ForNItems(SenseTraitsBase::ALL_TRAITS, Random::Number<size_t>(1, 4), [&](const auto& trait)
    {
        traits.push_back(SenseTraitsBase::DefaultNormalisation(trait));
    });
    unsigned hiddenLayers = Random::Number(size_t{ 0 }, traits.size());
    return std::make_shared<GeneSenseTraitsSelf>(std::move(traits), hiddenLayers, brainWidth);
}

GeneSenseTraitsSelf::GeneSenseTraitsSelf(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount)
    : GeneSenseTraitsBase(std::move(toDetect), hiddenLayers, outputCount, {})
{
    // No additional mutations
}

GeneSenseTraitsSelf::GeneSenseTraitsSelf(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, double dominance)
    : GeneSenseTraitsBase(std::move(toDetect), network, outputConnections, {}, dominance)
{
    // No additional mutations
}

json GeneSenseTraitsSelf::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork()) },
        {KEY_OUTPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetOutputConnections()) },
        {KEY_TRAIT_NORMALISERS, GetSerialisedTraitNormalisers()},
    };
}

void GeneSenseTraitsSelf::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.senses.push_back(std::make_shared<SenseTraitsSelf>(GetNetwork(), GetOutputConnections(), owner, std::vector(toDetect_)));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneSenseTraitsSelf::Copy() const
{
    return std::make_shared<GeneSenseTraitsSelf>(std::vector(toDetect_), GetNetwork(), GetOutputConnections(), GetDominance());
}
