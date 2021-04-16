#ifndef SENSESINE_H
#define SENSESINE_H

#include "Sense.h"

#include <JsonHelpers.h>

#include <nlohmann/json.hpp>

#include <optional>

class SenseSine : public Sense {
public:
    struct SineWave {
    public:
        double amplitude_;
        double frequency_;
        double x_ = 0.0;

        static nlohmann::json Serialise(const SineWave& wave);
        std::optional<SineWave> Deserialise(const nlohmann::json& wave);

    private:
        static const inline std::string KEY_AMPLITUDE = "Amplitude";
        static const inline std::string KEY_FREQUENCY = "Frequency";
        static const inline std::string KEY_X = "X";
    };

    SenseSine(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<SineWave>&& sineWaves);

    virtual std::string_view GetName() const override { return "SineWaves"; }
    virtual std::string GetDescription() const override;

    virtual void PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& entities, const UniverseParameters& universeParameters) const override;

private:
    std::vector<SineWave> sineWaves_;

    virtual void PrepareToPrime() override;
};


#endif // SENSESINE_H
