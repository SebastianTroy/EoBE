#include "GeneSenseSine.h"

#include "Trilobyte.h"
#include "Phenotype.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneSenseSine::Generate(unsigned brainWidth)
{
    return std::make_shared<GeneSenseSine>(Random::Number(1, 3), brainWidth);
}

GeneSenseSine::GeneSenseSine(unsigned inputCount, unsigned outputCount)
    : GeneSenseSine(std::make_shared<NeuralNetwork>(0, inputCount, NeuralNetwork::InitialWeights::PassThrough), std::make_shared<NeuralNetworkConnector>(NeuralNetworkConnector(inputCount, outputCount)), CreateRandomWaves(inputCount), Random::Number(0.0, 100.0))
{
}

GeneSenseSine::GeneSenseSine(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, std::vector<SenseSine::SineWave>&& sineWaves, double dominance)
    : GeneSenseBase(network, outputConnections, dominance)
    , sineWaves_(std::move(sineWaves))
{
    // Modify one
    AddMutation(BASE_WEIGHT, [&]()
    {
        if (!sineWaves_.empty()) {
            auto& wave = Random::Item(sineWaves_);
            wave = { Random::GaussianAdjustment(wave.amplitude_, 0.1), Random::GaussianAdjustment(wave.frequency_, 0.1) };
        }
    });

    // Swap
    AddMutation(0.25 * BASE_WEIGHT, [&]()
    {
        if (sineWaves_.size() > 1) {
            std::swap(Random::Item(sineWaves_), Random::Item(sineWaves_));
        }
    });

    // Shuffle
    AddMutation(0.05 * BASE_WEIGHT, [&]()
    {
        if (sineWaves_.size() > 1) {
            Random::Shuffle(sineWaves_);
        }
    });

    AddColumnInsertedAndRemovedMutations(
    // Add wave
    0.15 * BASE_WEIGHT, [&](unsigned index)
    {
        auto iter = sineWaves_.begin();
        std::advance(iter, index);
        sineWaves_.insert(iter, { Random::Gaussian(0.75, 0.1), Random::Gaussian(0.01, 0.1) });
    },
    // Remove wave
    0.15 * BASE_WEIGHT, [&](unsigned index)
    {
        auto iter = sineWaves_.begin();
        std::advance(iter, index);
        sineWaves_.erase(iter);
    });
}

json GeneSenseSine::Serialise() const
{
    json waves = json::array();
    for (const auto& sineWave : sineWaves_) {
        waves.push_back(SenseSine::SineWave::Serialise(sineWave));
    }
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork()) },
        {KEY_OUTPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetOutputConnections()) },
        {KEY_SINE_WAVES, waves},
    };
}

void GeneSenseSine::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.senses.push_back(std::make_shared<SenseSine>(GetNetwork(), GetOutputConnections(), owner, std::vector(sineWaves_)));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneSenseSine::Copy() const
{
    return std::make_shared<GeneSenseSine>(GetNetwork(), GetOutputConnections(), std::vector(sineWaves_), GetDominance());
}

std::vector<SenseSine::SineWave> GeneSenseSine::CreateRandomWaves(unsigned count)
{
    std::vector<SenseSine::SineWave> waves;
    for (unsigned i = 0; i < count ; ++i) {
        waves.push_back({ Random::Gaussian(0.75, 0.1), Random::Gaussian(0.01, 0.1) });
    }
    return waves;
}
