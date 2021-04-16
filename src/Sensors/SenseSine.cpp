#include "SenseSine.h"

#include <Algorithm.h>

#include <iomanip>

using namespace nlohmann;

json SenseSine::SineWave::Serialise(const SenseSine::SineWave& wave)
{
    return {
        {KEY_AMPLITUDE, wave.amplitude_},
        {KEY_FREQUENCY, wave.frequency_},
        {KEY_X, wave.x_},
    };
}

std::optional<SenseSine::SineWave> SenseSine::SineWave::Deserialise(const json& wave)
{
    if (JsonHelpers::ValidateJsonObject(wave, { {KEY_AMPLITUDE, json::value_t::number_float}, {KEY_FREQUENCY, json::value_t::number_float}, {KEY_X, json::value_t::number_float} })) {
        return { {wave.at(KEY_AMPLITUDE).get<double>(), wave.at(KEY_FREQUENCY).get<double>(), wave.at(KEY_X).get<double>()} };
    }
    return std::nullopt;
}

SenseSine::SenseSine(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<SenseSine::SineWave>&& sineWaves)
    : Sense(network, outputConnections, owner)
    , sineWaves_(sineWaves)
{
}

std::string SenseSine::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>This sense acts like an internal rythm, instead of detecting "
            "something external. Each input is its own sine wave, and the value"
            " is recalculated each simulation tick.</p>"
            "<p>A wave's amplitude is not limited, however the inputs are "
            "clamped to between -1 and 1, leading to squared off peaks.</p>";

    desc << "<p>Waves:<ol>";

    for (const SineWave& wave : sineWaves_) {
        desc << "<li> Amplitude " << wave.amplitude_ << ", " << "Frequency " << wave.frequency_ << "</li>";
    }

    desc << "</p></ol>";

    return desc.str();
}

void SenseSine::PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& /*entities*/, const UniverseParameters& /*universeParameters*/) const
{
    Tril::IterateBoth<double, SineWave>(inputs, sineWaves_, [](double& input, const SineWave& sine)
    {
        input = std::clamp(sine.amplitude_ * std::sin(sine.x_), -1.0, 1.0);
    });
}

void SenseSine::PrepareToPrime()
{
    for (auto& sine : sineWaves_) {
        sine.x_ += sine.frequency_;
    }
}
