#include "SenseTraitsBase.h"

#include "Trilobyte.h"

#include <Algorithm.h>

using namespace nlohmann;

std::string SenseTraitsBase::ToString(SenseTraitsBase::Trait trait)
{
    switch (trait) {
    case Trait::Red : return "Red";
    case Trait::Green : return "Green";
    case Trait::Blue : return "Blue";
    case Trait::Energy : return "Energy";
    case Trait::Age : return "Age";
    case Trait::Size : return "Size";
    case Trait::Distance : return "Distance";
    case Trait::Health : return "Health";
    case Trait::Presence : return "Presence";
    }
    assert(false && "SenseTraitsBase::ToString(Trait), invalid trait.");
    return "";
}

json SenseTraitsBase::TraitNormaliser::SerialiseTraitNormaliser(const SenseTraitsBase::TraitNormaliser& normaliser)
{
    return {
        {KEY_TRAIT, normaliser.trait},
        {KEY_CONVERTER, Tril::RangeConverter::Serialise(normaliser.range)},
    };
}

std::optional<SenseTraitsBase::TraitNormaliser> SenseTraitsBase::TraitNormaliser::DeserialiseTraitNormaliser(const json& normaliser)
{
    if (JsonHelpers::ValidateJsonObject(normaliser, { {KEY_TRAIT, json::value_t::number_unsigned}, {KEY_CONVERTER, json::value_t::object} })) {
        Trait trait = normaliser.at(KEY_TRAIT).get<Trait>();
        std::optional<Tril::RangeConverter> converter = Tril::RangeConverter::Deserialise(normaliser.at(KEY_CONVERTER));
        if (converter.has_value()) {
            return { {trait, converter.value()} };
        }
    }
    return std::nullopt;
}

SenseTraitsBase::TraitNormaliser SenseTraitsBase::DefaultNormalisation(const SenseTraitsBase::Trait& trait)
{
    switch (trait) {
    case Trait::Red :
        return { trait, { Tril::Range(0.0, 0.9), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Green :
        return { trait, { Tril::Range(0.0, 0.9), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Blue :
        return { trait, { Tril::Range(0.0, 0.9), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Energy :
        return { trait, { Tril::Range(0_j, 500_j), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Age :
        return { trait, { Tril::Range(0.0, 50'000.9), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Size :
        return { trait, { Tril::Range(0.0, 30.0), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Distance :
        return { trait, { Tril::Range(0.0, 100.0), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Health :
        return { trait, { Tril::Range(0.0, 100.0), Tril::Range(0.0, Random::Sign(1.0)) } };
    case Trait::Presence :
        return { trait, { Tril::Range(0.0, 10.0), Tril::Range(0.0, Random::Sign(1.0)) } };
    }
    assert(false && "Unimplemented Trait");
    return { Trait{ -1 }, { Tril::Range(0.0, 0.0), Tril::Range(0.0, Random::Sign(1.0)) }};
}

SenseTraitsBase::SenseTraitsBase(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, const Transform& transform, std::vector<TraitNormaliser>&& toDetect)
    : Sense(network, outputConnections, owner)
    , transform_(transform)
    , toDetect_(std::move(toDetect))
{
}

void SenseTraitsBase::PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& entities, const UniverseParameters& /*universeParameters*/) const
{
    // First adjust inputs for each entity detected
    FilterEntities(entities, [&](const Entity& entity)
    {
        Tril::IterateBoth<TraitNormaliser, double>(toDetect_, inputs, [&](const TraitNormaliser& norm, double& input)
        {
            input += GetTraitFrom(entity, norm.trait);
        });
    });
    // Then normalise each input according to its normalisation range
    Tril::IterateBoth<TraitNormaliser, double>(toDetect_, inputs, [&](const TraitNormaliser& norm, double& input)
    {
        input = norm.range.ConvertAndClamp(input);
    });
}

double SenseTraitsBase::GetTraitFrom(const Entity& target, Trait trait) const
{
    switch (trait) {
    case Trait::Red :
        return target.GetColour().redF();
    case Trait::Green :
        return target.GetColour().greenF();
    case Trait::Blue :
        return target.GetColour().blueF();
    case Trait::Energy :
        return target.GetEnergy();
    case Trait::Age :
        return target.GetAge();
    case Trait::Size :
        return target.GetRadius();
    case Trait::Distance : {
        Transform senseLocation = transform_ + owner_.GetTransform();
        return GetDistance({ senseLocation.x, senseLocation.y }, target.GetLocation());
    }
    case Trait::Health :
        if (const Trilobyte* targetTrilobyte = dynamic_cast<const Trilobyte*>(&target)) {
            return targetTrilobyte->GetHealth();
        } else {
            return 0.0;
        }
    case Trait::Presence :
        return 1.0;
    }
    assert(false && "Entity::GetTrait, unimplemented trait.");
    return 0.0;
}
