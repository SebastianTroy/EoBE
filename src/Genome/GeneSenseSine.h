#ifndef GENESINE_H
#define GENESINE_H

#include "GeneSenseBase.h"
#include "Sensors/SenseSine.h"

#include <NeuralNetwork.h>
#include <NeuralNetworkConnector.h>
#include <TypeName.h>

class GeneSenseSine : public GeneSenseBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneSenseSine>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneSenseSine(unsigned inputCount, unsigned outputCount);
    GeneSenseSine(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, std::vector<SenseSine::SineWave>&& sineWaves, double dominance);
    virtual ~GeneSenseSine() override {}

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_SINE_WAVES = "Waves";

    std::vector<SenseSine::SineWave> sineWaves_;

    static std::vector<SenseSine::SineWave> CreateRandomWaves(unsigned count);
};

#endif // GENESINE_H
