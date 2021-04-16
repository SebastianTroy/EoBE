#include "GeneSenseRandom.h"

#include "Phenotype.h"
#include "Sensors/SenseRandom.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneSenseRandom::Generate(unsigned brainWidth)
{
    return std::make_shared<GeneSenseRandom>(Random::Number(1, 3), brainWidth);
}

GeneSenseRandom::GeneSenseRandom(unsigned inputCount, unsigned outputCount)
    : GeneSenseRandom(std::make_shared<NeuralNetwork>(0, inputCount, NeuralNetwork::InitialWeights::PassThrough), std::make_shared<NeuralNetworkConnector>(NeuralNetworkConnector(inputCount, outputCount)), CreateRandomFilteredRandoms(inputCount), Random::Number(0.0, 100.0))
{
}

GeneSenseRandom::GeneSenseRandom(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, std::vector<SenseRandom::FilteredRandom>&& filteredRandoms, double dominance)
    : GeneSenseBase(network, outputConnections, dominance)
    , filteredRandoms_(std::move(filteredRandoms))
{
    // Modify one
    AddMutation(BASE_WEIGHT, [&]()
    {
        if (!filteredRandoms_.empty()) {
            auto& random = Random::Item(filteredRandoms_);
            random = { Random::GaussianAdjustment(random.min_, 0.1), Random::GaussianAdjustment(random.max_, 0.1), std::clamp(Random::GaussianAdjustment(random.beta_, 0.1), 0.0, 1.0) };
        }
    });

    // Shuffle
    AddMutation(0.05 * BASE_WEIGHT, [&]()
    {
        if (filteredRandoms_.size() > 1) {
            Random::Shuffle(filteredRandoms_);
        }
    });

    // Swap
    AddMutation(0.33 * BASE_WEIGHT, [&]()
    {
        if (filteredRandoms_.size() > 1) {
            std::swap(Random::Item(filteredRandoms_), Random::Item(filteredRandoms_));
        }
    });

    AddColumnInsertedAndRemovedMutations(
    // Add one
    0.5 * BASE_WEIGHT, [&](unsigned index)
    {
        auto iter = filteredRandoms_.begin();
        std::advance(iter, index);
        filteredRandoms_.insert(iter, { -1.0, 1.0, std::clamp(Random::Gaussian(0.5, 0.1), 0.0, 1.0) });
    },
    // Remove one
    0.5 * BASE_WEIGHT, [&](unsigned index)
    {
        auto iter = filteredRandoms_.begin();
        std::advance(iter, index);
        filteredRandoms_.erase(iter);
    });
}

json GeneSenseRandom::Serialise() const
{
    json randoms = json::array();
    for (const auto& rand : filteredRandoms_) {
        randoms.push_back(SenseRandom::FilteredRandom::Serialise(rand));
    }
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork()) },
        {KEY_OUTPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetOutputConnections()) },
        {KEY_RAND_ARRAY, randoms},
    };
}

void GeneSenseRandom::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.senses.push_back(std::make_shared<SenseRandom>(GetNetwork(), GetOutputConnections(), owner, std::vector(filteredRandoms_)));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneSenseRandom::Copy() const
{
    return std::make_shared<GeneSenseRandom>(GetNetwork(), GetOutputConnections(), std::vector(filteredRandoms_), GetDominance());
}

std::vector<SenseRandom::FilteredRandom> GeneSenseRandom::CreateRandomFilteredRandoms(unsigned count)
{
    std::vector<SenseRandom::FilteredRandom> randoms;
    for (unsigned i = 0; i < count; ++i) {
        randoms.push_back({ -1.0, 1.0, std::clamp(Random::Gaussian(0.5, 0.1), 0.0, 1.0) });
    }
    return randoms;
}
