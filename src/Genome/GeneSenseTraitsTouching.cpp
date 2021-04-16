#include "GeneSenseTraitsTouching.h"

#include "Phenotype.h"
#include "Sensors/SenseTraitsTouching.h"

#include <Random.h>
#include <JsonHelpers.h>

using namespace nlohmann;

std::shared_ptr<Gene> GeneSenseTraitsTouching::Generate(unsigned brainWidth)
{
    std::vector<SenseTraitsBase::TraitNormaliser> traits;
    // Between 1 - 4 traits
    Random::ForNItems(SenseTraitsBase::ALL_TRAITS, Random::Number<size_t>(1, 4), [&](const auto& trait)
    {
        traits.push_back(SenseTraitsBase::DefaultNormalisation(trait));
    });
    unsigned hiddenLayers = Random::Number(size_t{ 0 }, traits.size());
    Transform transform = { 0, Random::Number(0.0, 20.0), Random::Gaussian(0.0, Tril::Pi) };
    return std::make_shared<GeneSenseTraitsTouching>(std::move(traits), hiddenLayers, brainWidth, transform);
}

// TODO also see https://stackoverflow.com/questions/52191576/nlohmann-json-get-with-type-deduction
//std::shared_ptr<Gene> GeneSenseTraitsTouching::Deserialise(const json& serialised)
//{
//    if (JsonHelpers::ValidateJsonObject(serialised, { {KEY_DOMINANCE, json::value_t::number_float},
//                                                      {KEY_NETWORK, json::value_t::object},
//                                                      {KEY_OUTPUT_CONNECTIONS, json::value_t::object},
//                                                      {KEY_TRANSFORM, json::value_t::object},
//                                                      {KEY_TRAIT_NORMALISERS, json::value_t::array} })) {
//        std::vector<SenseTraitsBase::TraitNormaliser> toDetect;
//        if (JsonHelpers::ValidateJsonArray(serialised.at(KEY_TRAIT_NORMALISERS), json::value_t::object)) {
//            toDetect = JsonHelpers::ParseJsonArray<SenseTraitsBase::TraitNormaliser>(serialised.at(KEY_TRAIT_NORMALISERS));
//        }
//        std::shared_ptr<NeuralNetwork> network;
//        std::shared_ptr<NeuralNetworkConnector> outputConnections;
//        Transform transform;
//        double dominance = serialised.at(KEY_DOMINANCE).get<double>();
//        return std::make_shared<GeneSenseTraitsTouching>(std::move(toDetect), network, outputConnections, transform, dominance);
//    }
//    return nullptr;
//}

GeneSenseTraitsTouching::GeneSenseTraitsTouching(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount, const Transform& transform)
    : GeneSenseTraitsBase(std::move(toDetect), hiddenLayers, outputCount, transform)
{
    AddMutations();
}

GeneSenseTraitsTouching::GeneSenseTraitsTouching(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Transform& transform, double dominance)
    : GeneSenseTraitsBase(std::move(toDetect), network, outputConnections, transform, dominance)
{
    AddMutations();
}

json GeneSenseTraitsTouching::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork()) },
        {KEY_OUTPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetOutputConnections()) },
        {KEY_TRANSFORM, Transform::Serialise(transform_)},
        {KEY_TRAIT_NORMALISERS, GetSerialisedTraitNormalisers()},
    };
}

void GeneSenseTraitsTouching::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.senses.push_back(std::make_shared<SenseTraitsTouching>(GetNetwork(), GetOutputConnections(), owner, transform_, std::vector(toDetect_)));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneSenseTraitsTouching::Copy() const
{
    return std::make_shared<GeneSenseTraitsTouching>(std::vector(toDetect_), GetNetwork(), GetOutputConnections(), transform_, GetDominance());
}

void GeneSenseTraitsTouching::AddMutations()
{
    /*
     * FIXME currently the mutations for this class are all handled by the
     * mutate transform mutation in the SenseTraitsBase class, however this
     * gene ought to have some notion of a Point that it mutates seperately to
     * the transform.
     */
}
