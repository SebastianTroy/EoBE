#ifndef GENEEFFECTORPROBOSCISMOUTH_H
#define GENEEFFECTORPROBOSCISMOUTH_H

#include "GeneEffectorBase.h"
#include <TypeName.h>

class GeneEffectorProboscisMouth : public GeneEffectorBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneEffectorProboscisMouth>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneEffectorProboscisMouth(unsigned inputCount, double proboscisLength);
    GeneEffectorProboscisMouth(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance, double proboscisLength);
    ~GeneEffectorProboscisMouth() override;

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_LENGTH = "Length";

    constexpr static double minProboscisLength_ = 1.0;
    double proboscisLength_;

    void AddMutations();
};

#endif // GENEEFFECTORPROBOSCISMOUTH_H
