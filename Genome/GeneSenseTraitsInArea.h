#ifndef GENESENSEENTITIESINAREA_H
#define GENESENSEENTITIESINAREA_H

#include "GeneSenseTraitsBase.h"

class GeneSenseTraitsInArea final : public GeneSenseTraitsBase {
public:
    GeneSenseTraitsInArea(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount, const Transform& transform, double radius);
    GeneSenseTraitsInArea(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Transform& transform, double radius, double dominance, double mutationProbability);
    virtual ~GeneSenseTraitsInArea() override {}

    virtual std::shared_ptr<Gene> Mutate() const override;
    virtual void ExpressGene(Swimmer& owner, Phenotype& target) const override;

private:
    const double radius_;
};

#endif // GENESENSEENTITIESINAREA_H