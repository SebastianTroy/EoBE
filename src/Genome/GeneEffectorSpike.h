#ifndef GENEEFFECTORSPIKE_H
#define GENEEFFECTORSPIKE_H

#include "GeneEffectorBase.h"
#include <TypeName.h>

class GeneEffectorSpike : public GeneEffectorBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneEffectorSpike>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneEffectorSpike(unsigned inputCount, double bearing, double length);
    GeneEffectorSpike(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& inputConnections, double dominance, double bearing, double length);
    ~GeneEffectorSpike() override;

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_BEARING = "Bearing";
    static const inline std::string KEY_LENGTH = "Length";

    double bearing_;
    double length_;

    void AddMutations();
};

#endif // GENEEFFECTORSPIKE_H
