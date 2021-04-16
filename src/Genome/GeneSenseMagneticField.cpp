#include "GeneSenseMagneticField.h"

#include "Phenotype.h"
#include "Sensors/SenseMagneticField.h"

using namespace nlohmann;

std::shared_ptr<Gene> GeneSenseMagneticField::Generate(unsigned brainWidth)
{
    return std::make_shared<GeneSenseMagneticField>(Random::Number(0, 2), brainWidth, Point{ Random::Number(-1000.0, 1000.0), Random::Number(-1000.0, 1000.0) });
}

GeneSenseMagneticField::GeneSenseMagneticField(unsigned hiddenLayers, unsigned outputCount, const Point& point)
    : GeneSenseMagneticField(std::make_shared<NeuralNetwork>(hiddenLayers, 3, NeuralNetwork::InitialWeights::PassThrough), std::make_shared<NeuralNetworkConnector>(3, outputCount), point, Tril::RangeConverter({0, 1000}, {0, 0.3}), {-0.1, 0.1}, {-0.1, 0.1}, Random::Number(0.0, 100.0))
{
}

GeneSenseMagneticField::GeneSenseMagneticField(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Point& target, const Tril::RangeConverter& distanceNormaliser, const Tril::Range<double>& frontBackNormaliser, const Tril::Range<double>& leftRightNormaliser, double dominance)
    : GeneSenseBase(network, outputConnections, dominance)
    , target_(target)
    , distanceNormaliser_(distanceNormaliser)
    , frontBackNormaliser_(frontBackNormaliser)
    , leftRightNormaliser_(leftRightNormaliser)
{
    // Move target
    AddMutation(BASE_WEIGHT, [&]()
    {
        target_ = { Random::Gaussian(target.x, 5.0), Random::Gaussian(target.y, 5.0) };
    });

    // Move target a lot
    AddMutation(0.05 * BASE_WEIGHT, [&]()
    {
        target_ = { Random::Gaussian(target.x, 100.0), Random::Gaussian(target.y, 100.0) };
    });

    // Adjust range within which the sence can discern differences in distance
    AddMutation(BASE_WEIGHT, [&]()
    {
        Tril::Range<double> from = distanceNormaliser_.GetFrom();
        from = { Random::Gaussian(from.First(), 5.0), Random::Gaussian(from.Last(), 5.0) };
        distanceNormaliser_ = { from, distanceNormaliser_.GetTo() };
    });

    // Adjust neuron activation based on range
    AddMutation(BASE_WEIGHT, [&]()
    {
        Tril::Range<double> to = distanceNormaliser_.GetTo();
        to = { std::clamp(Random::Gaussian(to.First(), 0.05), -1.0, 1.0), std::clamp(Random::Gaussian(to.Last(), 0.05), -1.0, 1.0) };
        distanceNormaliser_ = { distanceNormaliser_.GetFrom(), to };
    });

    // Big adjustment range within which the sence can discern differences in distance
    AddMutation(0.05 * BASE_WEIGHT, [&]()
    {
        Tril::Range<double> from = distanceNormaliser_.GetFrom();
        from = { Random::Gaussian(from.First(), 100.0), Random::Gaussian(from.Last(), 100.0) };
        distanceNormaliser_ = { from, distanceNormaliser_.GetTo() };
    });

    // Adjust neuron activation based on front/back orientation
    AddMutation(BASE_WEIGHT, [&]()
    {
        Tril::Range<double> to = frontBackNormaliser_;
        to = { std::clamp(Random::Gaussian(to.First(), 0.05), -1.0, 1.0), std::clamp(Random::Gaussian(to.Last(), 0.05), -1.0, 1.0) };
        frontBackNormaliser_ = to;
    });

    // Adjust neuron activation based on left/right orientation
    AddMutation(BASE_WEIGHT, [&]()
    {
        Tril::Range<double> to = leftRightNormaliser_;
        to = { std::clamp(Random::Gaussian(to.First(), 0.05), -1.0, 1.0), std::clamp(Random::Gaussian(to.Last(), 0.05), -1.0, 1.0) };
        leftRightNormaliser_ = to;
    });
}

json GeneSenseMagneticField::Serialise() const
{
    return {
        {KEY_DOMINANCE, GetDominance()},
        {KEY_NETWORK, NeuralNetwork::Serialise(GetNetwork()) },
        {KEY_OUTPUT_CONNECTIONS, NeuralNetworkConnector::Serialise(GetOutputConnections()) },
        {KEY_TARGET, ::Serialise(target_)},
        {KEY_DISTANCE, Tril::RangeConverter::Serialise(distanceNormaliser_)},
        {KEY_FRONT_BACK, Tril::Range<double>::Serialise(frontBackNormaliser_)},
        {KEY_LEFT_RIGHT, Tril::Range<double>::Serialise(leftRightNormaliser_)},
    };
}

void GeneSenseMagneticField::ExpressGene(Trilobyte& owner, Phenotype& target) const
{
    target.senses.push_back(std::make_shared<SenseMagneticField>(GetNetwork(), GetOutputConnections(), owner, target_, distanceNormaliser_, frontBackNormaliser_, leftRightNormaliser_));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneSenseMagneticField::Copy() const
{
    return std::make_shared<GeneSenseMagneticField>(GetNetwork(), GetOutputConnections(), target_, distanceNormaliser_, frontBackNormaliser_, leftRightNormaliser_, GetDominance());
}
