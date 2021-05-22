#include "EffectorSpringTail.h"

#include "Trilobyte.h"

#include <iomanip>

EffectorSpringTail::EffectorSpringTail(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, Trilobyte& owner, const Tril::RangeConverter& neuronExcitementToChargeRate, const Energy& storedEnergyCap, const double& triggerThreshold)
    : Effector(network, inputConnections, owner)
    , neuronExcitementToChargeRate_(neuronExcitementToChargeRate)
    , storedEnergyCap_(storedEnergyCap)
    , triggerThreshold_(triggerThreshold)
    , storedEnergy_(0_j)
{
}

EffectorSpringTail::~EffectorSpringTail()
{
}

std::string EffectorSpringTail::GetDescription() const
{
    std::stringstream desc;
    desc << std::fixed << std::setprecision(2);

    desc << "<p>The spring tail allows trilobytes to efficiently store up energy "
            "release later as a burst of forward acceleration.</p>";
    desc << "<p>Energy stored: " << (storedEnergy_ / 1_uj) << "uj/ " << (storedEnergyCap_ / 1_uj) << "uj.</p>";
    desc << "<p>Trigger Threshold: " << triggerThreshold_ << "</p>";

    return desc.str();
}

void EffectorSpringTail::Draw(QPainter& /*paint*/) const
{
    // TODO
}

Energy EffectorSpringTail::PerformActions(const std::vector<double>& actionValues, EntityContainerInterface& /*entities*/, const UniverseParameters& /*universeParameters*/)
{
    const Energy chargeRate = std::clamp(neuronExcitementToChargeRate_.Convert(actionValues.at(0)), 0_uj, storedEnergyCap_ - storedEnergy_);
    const double& triggerLevel = actionValues.at(1);

    if (triggerLevel > triggerThreshold_ && storedEnergy_ == storedEnergyCap_) {
        owner_.AdjustVelocity(storedEnergy_ / 25_uj);
        storedEnergy_ = 0_uj;
    }

    storedEnergy_ += chargeRate;
    return chargeRate;
}
