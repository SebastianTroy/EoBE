#ifndef GENEEFFECTORFILTERMOUTH_H
#define GENEEFFECTORFILTERMOUTH_H

#include "GeneEffectorBase.h"
#include <TypeName.h>

class GeneEffectorFilterMouth : public GeneEffectorBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneEffectorFilterMouth>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneEffectorFilterMouth(unsigned hiddenLayers, unsigned inputCount);
    GeneEffectorFilterMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance);
    virtual ~GeneEffectorFilterMouth();

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;
};

#endif // GENEEFFECTORFILTERMOUTH_H
