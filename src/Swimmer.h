#ifndef SWIMMER_H
#define SWIMMER_H

#include "Entity.h"
#include "Genome/Genome.h"
#include "Sensors/Sense.h"
#include "Effectors/Effector.h"

#include <NeuralNetwork.h>

#include <memory>

class Swimmer : public Entity, public std::enable_shared_from_this<Swimmer> {
public:
    Swimmer(Energy energy, const Transform& transform, std::shared_ptr<Genome> genome);
    Swimmer(Energy energy, const Transform& transform, std::shared_ptr<Genome> genome, std::shared_ptr<Swimmer>&& parent);
    virtual ~Swimmer() override;

    virtual std::string_view GetName() const override { return "Swimmer"; }
    virtual std::string_view GetDescription() const override;

    std::shared_ptr<Entity> GiveBirth(const std::shared_ptr<Genome>& other);

    const std::shared_ptr<NeuralNetwork>& InspectBrain() { return brain_; };
    const std::vector<std::shared_ptr<Sense>>& InspectSenses() { return senses_; };
    const std::vector<std::shared_ptr<Effector>>& InspectEffectors() { return effectors_; };
    const std::shared_ptr<Genome>& InspectGenome() { return genome_; };

    uint64_t GetGeneration() const { return generation_; }
    const Energy& GetBaseMetabolism() const { return baseMetabolism_; }
    double GetHealth() const { return health_; }
    unsigned GetEggsLayedCount() const { return eggsLayed_; }
    unsigned GetTotalDescendantsCount(unsigned generation) const { return totalDescentantCounts_.count(generation) ? totalDescentantCounts_.at(generation) : 0; }
    unsigned GetLivingDescendantsCount(unsigned generation) const { return extantDescentantCounts_.count(generation) ? extantDescentantCounts_.at(generation) : 0; }
    unsigned GetTotalDescendantsCount() const;
    unsigned GetLivingDescendantsCount() const;
    uint64_t GetGeneMutationCount() const { return genome_->GetGeneMutationCount(); }
    uint64_t GetChromosomeMutationCount() const { return genome_->GetChromosomeMutationCount(); }


    void AdjustVelocity(double adjustment);
    void AdjustBearing(double adjustment);
    void ApplyDamage(double damage) { health_ -= std::min(health_, damage); }

protected:
    std::shared_ptr<Swimmer> closestLivingAncestor_;

    virtual void TickImpl(EntityContainerInterface& container, const UniverseParameters& universeParameters) override final;
    virtual void DrawImpl(QPainter& paint) override final;

private:
    const uint64_t generation_;
    const Energy baseMetabolism_;

    double health_;

    std::shared_ptr<Genome> genome_;
    std::shared_ptr<NeuralNetwork> brain_;
    std::vector<std::shared_ptr<Sense>> senses_;
    std::vector<std::shared_ptr<Effector>> effectors_;
    std::vector<double> brainValues_;

    unsigned eggsLayed_;
    // <relative generation (where children of this are gen 1), count>
    std::map<unsigned, unsigned> totalDescentantCounts_;
    std::map<unsigned, unsigned> extantDescentantCounts_;

    Swimmer(Energy energy, const Transform& transform, std::shared_ptr<Genome> genome, const Phenotype& phenotype, std::shared_ptr<Swimmer>&& mother);

    virtual std::vector<Property> CollectProperties() const override;

    std::shared_ptr<Swimmer> FindClosestLivingAncestor() const;

    void DescendantBorn(unsigned generation);
    void DescendantDied(unsigned generation);
};

#endif // SWIMMER_H
