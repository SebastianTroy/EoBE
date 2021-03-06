#ifndef GENESINE_H
#define GENESINE_H

#include "GeneSenseBase.h"
#include "NeuralNetwork.h"
#include "NeuralNetworkConnector.h"
#include "Sensors/SenseSine.h"

class GeneSenseSine : public GeneSenseBase {
public:
    GeneSenseSine(unsigned inputCount, unsigned outputCount);
    GeneSenseSine(const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, std::vector<SenseSine::SineWave>&& sineWaves, double dominance);
    virtual ~GeneSenseSine() override {}

    virtual std::string Name() const override { return "GeneSenseSine"; }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Swimmer& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_SINE_WAVES = "Waves";

    std::vector<SenseSine::SineWave> sineWaves_;

    static std::vector<SenseSine::SineWave> CreateRandomWaves(unsigned count);
};

#endif // GENESINE_H
