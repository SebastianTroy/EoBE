#ifndef GENEEFFECTORSPRINGTAIL_H
#define GENEEFFECTORSPRINGTAIL_H

#include <RangeConverter.h>
#include <TypeName.h>

#include "GeneEffectorBase.h"

class GeneEffectorSpringTail : public GeneEffectorBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneEffectorSpringTail>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneEffectorSpringTail(unsigned hiddenLayers, unsigned inputCount, const Tril::RangeConverter& neuronExcitementToChargeRate, const Energy& storedEnergyCap, const double& triggerThreshold);
    GeneEffectorSpringTail(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance, const Tril::RangeConverter& neuronExcitementToChargeRate, const Energy& storedEnergyCap, const double& triggerThreshold);
    ~GeneEffectorSpringTail() override;

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_CHARGE_RATE = "ChargeRate";
    static const inline std::string KEY_ENERGY_CAP = "EnergyCap";
    static const inline std::string KEY_TRIGGER = "Trigger";

    Tril::RangeConverter neuronExcitementToChargeRate_;
    Energy storedEnergyCap_;
    double triggerThreshold_;

    void AddMutations();
};

#endif // GENEEFFECTORSPRINGTAIL_H
