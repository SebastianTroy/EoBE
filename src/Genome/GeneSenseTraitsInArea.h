#ifndef GENESENSEENTITIESINAREA_H
#define GENESENSEENTITIESINAREA_H

#include "GeneSenseTraitsBase.h"
#include <TypeName.h>

class GeneSenseTraitsInArea final : public GeneSenseTraitsBase {
public:
    static inline std::string Name() { return std::string(Tril::TypeName<GeneSenseTraitsInArea>()); }
    static std::shared_ptr<Gene> Generate(unsigned brainWidth);
    static std::shared_ptr<Gene> Deserialise(const nlohmann::json& serialised);

    GeneSenseTraitsInArea(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount, const Transform& transform, double radius);
    GeneSenseTraitsInArea(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Transform& transform, double radius, double dominance);
    virtual ~GeneSenseTraitsInArea() override {}

    virtual std::string GetName() const override { return Name(); }
    virtual nlohmann::json Serialise() const override;
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const override;

protected:
    virtual std::shared_ptr<Gene> Copy() const override;

private:
    static const inline std::string KEY_RADIUS = "Radius";

    double radius_;

    void AddMutations();
};

#endif // GENESENSEENTITIESINAREA_H
