#ifndef SENSERANDOM_H
#define SENSERANDOM_H

#include "Sense.h"

#include <JsonHelpers.h>

#include <nlohmann/json.hpp>

#include <optional>

class SenseRandom : public Sense {
public:
    struct FilteredRandom {
    public:
        double min_;
        double max_;
        double beta_; // i.e. Low pass filter

        static nlohmann::json Serialise(const FilteredRandom& wave);
        std::optional<FilteredRandom> Deserialise(const nlohmann::json& wave);

    private:
        static const inline std::string KEY_MIN = "Min";
        static const inline std::string KEY_MAX = "Max";
        static const inline std::string KEY_BETA = "Beta";
    };

    SenseRandom(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<FilteredRandom>&& filteredRandoms);
    virtual ~SenseRandom() override {}

    virtual std::string_view GetName() const override { return "Random"; }
    virtual std::string GetDescription() const override;

    virtual void PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& entities, const UniverseParameters& universeParameters) const override;

private:
    const std::vector<FilteredRandom> filteredRandoms_;
    std::vector<double> last_;

    virtual void PrepareToPrime() override;
};

#endif // SENSERANDOM_H
