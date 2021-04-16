#ifndef GENEEFFECTORTAIL_H
#define GENEEFFECTORTAIL_H

#include "GeneEffectorBase.h"
#include <TypeName.h>

#include <memory>

class GeneEffectorTail : public GeneEffectorBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneEffectorTail>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneEffectorTail(unsigned hiddenLayers, unsigned inputCount);
    GeneEffectorTail(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance);
    virtual ~GeneEffectorTail() {}

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;
};

#endif // GENEEFFECTORTAIL_H
