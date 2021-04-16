#include "SenseRandom.h"


#include <Algorithm.h>
#include <Random.h>

#include <iomanip>

using namespace nlohmann;

json SenseRandom::FilteredRandom::Serialise(const SenseRandom::FilteredRandom& wave)
{
    return {
        {KEY_MIN, wave.min_},
        {KEY_MAX, wave.max_},
        {KEY_BETA, wave.beta_},
    };
}

std::optional<SenseRandom::FilteredRandom> SenseRandom::FilteredRandom::Deserialise(const json& wave)
{
    if (JsonHelpers::ValidateJsonObject(wave, { {KEY_MIN, json::value_t::number_float}, {KEY_MAX, json::value_t::number_float}, {KEY_BETA, json::value_t::number_float} })) {
        return { {wave.at(KEY_MIN).get<double>(), wave.at(KEY_MAX).get<double>(), wave.at(KEY_BETA).get<double>()} };
    }
    return std::nullopt;
}

SenseRandom::SenseRandom(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Trilobyte& owner, std::vector<FilteredRandom>&& filteredRandoms)
    : Sense(network, outputConnections, owner)
    , filteredRandoms_(std::move(filteredRandoms))
    , last_(filteredRandoms_.size(), 0.0)
{
}

std::string SenseRandom::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>This sense acts as a source of randomness, each tick, a random"
            "value is selected for each input from within the input's range. "
            "The value is filtered however to smooth out the changes and "
            "prevent anything too abrupt. The value is calculated as follows:"
            "</p>"
            "<p>input = ((1 - beta) * previousValue) + (beta * randomValue)</p>"
            "<p>Where beta is a value between 0 and 1. The lower the beta value"
            ", the more smoothing there is.</p>";

    desc << "<p>Entropy sources:<ol>";

    for (const FilteredRandom& rand : filteredRandoms_) {
        desc << "<li> Range " << rand.min_ << " -> " << rand.max_ << ", " << "Beta " << rand.beta_ << "</li>";
    }

    desc << "</p></ol>";

    return desc.str();
}

void SenseRandom::PrimeInputs(std::vector<double>& inputs, const EntityContainerInterface& /*entities*/, const UniverseParameters& /*universeParameters*/) const
{
    Tril::IterateBoth<double, double>(inputs, last_, [](double& input, const double& last)
    {
        input = last;
    });
}

void SenseRandom::PrepareToPrime()
{
    Tril::IterateBoth<double, FilteredRandom>(last_, filteredRandoms_, [](double& previous, const FilteredRandom& filter) {
        previous = std::clamp(previous + (filter.beta_ * (Random::Number(filter.min_, filter.max_) - previous)), -1.0, 1.0);
    });
}
