#ifndef SENSETRAITSBASE_H
#define SENSETRAITSBASE_H

#include "Sense.h"

#include <Energy.h>
#include <Transform.h>
#include <JsonHelpers.h>
#include <RangeConverter.h>

#include <nlohmann/json.hpp>

#include <functional>

class Entity;

class SenseTraitsBase : public Sense {
public:
    enum class Trait {
        Red,
        Green,
        Blue,
        Energy,
        Age,
        Size,
        Distance,
        Health,
        Presence,
    };

    static constexpr std::array ALL_TRAITS = { Trait::Red, Trait::Green, Trait::Blue, Trait::Energy, Trait::Age, Trait::Size, Trait::Distance, Trait::Health, Trait::Presence };

    static std::string ToString(Trait trait);

    /**
     * Used to convert the combined values of all detected traits into a value
     * between 0.0 -> 1.0. This allows senses to evolve their own sensitivities
     * to various traits, and to decide which traits they sense.
     *
     * TODO consider allowing non-linear normalisation options
     */
    struct TraitNormaliser {
    public:
        Trait trait;
        Tril::RangeConverter range;

        static nlohmann::json SerialiseTraitNormaliser(const TraitNormaliser& normaliser);
        static std::optional<TraitNormaliser> DeserialiseTraitNormaliser(const nlohmann::json& normaliser);
    private:
        static const inline std::string KEY_TRAIT = "Trait";
        static const inline std::string KEY_CONVERTER = "Converter";
    };

    /**
     * Chooses a reasonable range for each trait, such that it is likely that
     * the value will be normalised to a number between 0.0 -> 1.0 for a single
     * entity being detected at full strength.
     */
    static TraitNormaliser DefaultNormalisation(const Trait& trait);

    SenseTraitsBase(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, const Transform& transform, std::vector<TraitNormaliser>&& toDetect);

    virtual void PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& entities, const UniverseParameters& universeParameters) const override final;

    const std::vector<TraitNormaliser>& GetDetectableTraits() const { return toDetect_; }

protected:
    const Transform transform_;

private:
    const std::vector<TraitNormaliser> toDetect_;

    /**
     * This allows senses to respond to any trait easily, it is up to the caller
     * to normalise the value.
     */
    double GetTraitFrom(const Entity& target, Trait trait) const;

    virtual void FilterEntities(const EntityContainerInterface& entities, const std::function<void(const Entity& e)>& forEachEntity) const = 0;
};

#endif // SENSETRAITSBASE_H
