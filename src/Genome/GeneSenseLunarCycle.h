#ifndef GENESENSELUNARCYCLE_H
#define GENESENSELUNARCYCLE_H

#include "GeneSenseBase.h"
#include <TypeName.h>

class GeneSenseLunarCycle : public GeneSenseBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneSenseLunarCycle>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneSenseLunarCycle(unsigned hiddenLayers, unsigned outputCount);
    GeneSenseLunarCycle(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, double dominance);
    virtual ~GeneSenseLunarCycle() override;

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;
};

#endif // GENESENSELUNARCYCLE_H
