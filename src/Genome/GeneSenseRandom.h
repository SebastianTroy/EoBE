#ifndef GENESENSERANDOM_H
#define GENESENSERANDOM_H

#include "GeneSenseBase.h"
#include "Sensors/SenseRandom.h"

#include <NeuralNetwork.h>
#include <NeuralNetworkConnector.h>
#include <TypeName.h>

class GeneSenseRandom : public GeneSenseBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneSenseRandom>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneSenseRandom(unsigned inputCount, unsigned outputCount);
    GeneSenseRandom(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, std::vector<SenseRandom::FilteredRandom>&& filteredRandoms, double dominance);
    virtual ~GeneSenseRandom() override {}

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_RAND_ARRAY = "Randoms";

    std::vector<SenseRandom::FilteredRandom> filteredRandoms_;

    static std::vector<SenseRandom::FilteredRandom> CreateRandomFilteredRandoms(unsigned count);
};

#endif // GENESENSERANDOM_H
