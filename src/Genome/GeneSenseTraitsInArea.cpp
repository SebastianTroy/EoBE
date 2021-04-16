#include "GeneSenseTraitsInArea.h"

#include "Phenotype.h"
#include "Sensors/SenseTraitsInArea.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneSenseTraitsInArea::Generate(unsigned brainWidth)
{
    std::vector<SenseTraitsBase::TraitNormaliser> traits;
    // Between 1 - 4 traits
    Random::ForNItems(SenseTraitsBase::ALL_TRAITS, Random::Number<size_t>(1, 4), [&](const auto& trait)
    {
        traits.push_back(SenseTraitsBase::DefaultNormalisation(trait));
    });
    unsigned hiddenLayers = Random::Number(size_t{ 0 }, traits.size());
    double radius = Random::Number(5.0, 30.0);
    Transform transform = { 0, Random::Number(0.0, radius), Random::Bearing() };
    return std::make_shared<GeneSenseTraitsInArea>(std::move(traits), hiddenLayers, brainWidth, transform, radius);
}

GeneSenseTraitsInArea::GeneSenseTraitsInArea(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount, const Transform& transform, double radius)
    : GeneSenseTraitsBase(std::move(toDetect), hiddenLayers, outputCount, transform)
    , radius_(radius)
{
    AddMutations();
}

GeneSenseTraitsInArea::GeneSenseTraitsInArea(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Transform& transform, double radius, double dominance)
    : GeneSenseTraitsBase(std::move(toDetect), network, outputConnections, transform, dominance)
    , radius_(radius)
{
    AddMutations();
}

json GeneSenseTraitsInArea::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork()) },
        {KEY_OUTPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetOutputConnections()) },
        {KEY_TRANSFORM, Transform::Serialise(transform_)},
        {KEY_TRAIT_NORMALISERS, GetSerialisedTraitNormalisers()},
        {KEY_RADIUS, radius_},
    };
}

void GeneSenseTraitsInArea::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.senses.push_back(std::make_shared<SenseTraitsInArea>(GetNetwork(), GetOutputConnections(), owner, radius_, transform_, std::vector(toDetect_)));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneSenseTraitsInArea::Copy() const
{
    return std::make_shared<GeneSenseTraitsInArea>(std::vector(toDetect_), GetNetwork(), GetOutputConnections(), transform_, radius_, GetDominance());

}

void GeneSenseTraitsInArea::AddMutations()
{
    AddMutation(BASE_WEIGHT, [&]
    {
        radius_ = std::max(0.1, Random::GaussianAdjustment(radius_, 1.0));
    });
}
