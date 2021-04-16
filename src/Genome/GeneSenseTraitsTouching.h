#ifndef GENESENSEENTITIESTOUCHING_H
#define GENESENSEENTITIESTOUCHING_H

#include "GeneSenseTraitsBase.h"
#include <TypeName.h>

class GeneSenseTraitsTouching final : public GeneSenseTraitsBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneSenseTraitsTouching>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneSenseTraitsTouching(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount, const Transform& transform);
    GeneSenseTraitsTouching(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Transform& transform, double dominance);
    virtual ~GeneSenseTraitsTouching() override {}

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    void AddMutations();
};

#endif // GENESENSEENTITIESTOUCHING_H
