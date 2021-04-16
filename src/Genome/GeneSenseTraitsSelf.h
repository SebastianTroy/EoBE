#ifndef GENESENSETRAITSSELF_H
#define GENESENSETRAITSSELF_H

#include "GeneSenseTraitsBase.h"
#include <TypeName.h>

class GeneSenseTraitsSelf : public GeneSenseTraitsBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneSenseTraitsSelf>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneSenseTraitsSelf(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount);
    GeneSenseTraitsSelf(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, double dominance);
    virtual ~GeneSenseTraitsSelf() {}

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;
};

#endif // GENESENSETRAITSSELF_H
