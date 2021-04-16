#ifndef GENEBRAIN_H
#define GENEBRAIN_H

#include "Gene.h"

#include <NeuralNetwork.h>
#include <TypeName.h>

#include <memory>

class GeneBrain : public Gene {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneBrain>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneBrain(unsigned layerCount, unsigned width);
    GeneBrain(const std::shared_ptr<NeuralNetwork>& brain, double dominance);

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_BRAIN = "Brain";

    std::shared_ptr<NeuralNetwork> brain_;
};

#endif // GENEBRAIN_H
