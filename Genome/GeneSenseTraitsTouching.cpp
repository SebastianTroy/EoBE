#include "GeneSenseTraitsTouching.h"

#include "Phenotype.h"
#include "Sensors/SenseTraitsTouching.h"

GeneSenseTraitsTouching::GeneSenseTraitsTouching(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, unsigned hiddenLayers, unsigned outputCount, const Transform& transform)
    : GeneSenseTraitsBase(std::move(toDetect), hiddenLayers, outputCount, transform)
{
    AddMutations();
}

GeneSenseTraitsTouching::GeneSenseTraitsTouching(std::vector<SenseTraitsBase::TraitNormaliser>&& toDetect, const std::shared_ptr<NeuralNetwork>& network, const std::shared_ptr<NeuralNetworkConnector>& outputConnections, const Transform& transform, double dominance)
    : GeneSenseTraitsBase(std::move(toDetect), network, outputConnections, transform, dominance)
{
    AddMutations();
}

void GeneSenseTraitsTouching::ExpressGene(Swimmer& owner, Phenotype& target) const
{
    target.senses.push_back(std::make_shared<SenseTraitsTouching>(GetNetwork(), GetOutputConnections(), owner, transform_, std::vector(toDetect_)));
    target.baseMetabolism += GetMetabolicCost();
}

std::shared_ptr<Gene> GeneSenseTraitsTouching::Copy() const
{
    return std::make_shared<GeneSenseTraitsTouching>(std::vector(toDetect_), GetNetwork(), GetOutputConnections(), transform_, GetDominance());
}

void GeneSenseTraitsTouching::AddMutations()
{
    /*
     * FIXME currently the mutations for this class are all handles by the
     * mutate transform mutation in the SenseTraitsBase class, however this
     * gene ought to have some notion of a Point that it mutates seperately to
     * the transform.
     */
}
