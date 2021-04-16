#ifndef GENE_H
#define GENE_H

#include <Random.h>

#include <nlohmann/json.hpp>

#include <memory>
#include <functional>

class Trilobyte;
struct Phenotype;

class Gene {
public:
    Gene(double dominance);
    Gene(const Gene& other) = delete;
    virtual ~Gene();

    // TODO move these static heleprs away, they shouldn't live in this class
    static std::shared_ptr<Gene>& GetMostDominant(std::pair<std::shared_ptr<Gene>, std::shared_ptr<Gene>>& alleles);
    static const std::shared_ptr<Gene>& GetMostDominant(const std::pair<std::shared_ptr<Gene>, std::shared_ptr<Gene> >& alleles);
    static std::shared_ptr<Gene>& GetRandom(std::pair<std::shared_ptr<Gene>, std::shared_ptr<Gene>>& alleles);
    static bool Empty(const std::pair<std::shared_ptr<Gene>, std::shared_ptr<Gene>>& alleles) { return !(alleles.first || alleles.second); };

    /**
     * Name used with serialise/deserialise to uniquely identify the class.
     */
    virtual std::string GetName() const = 0;
    virtual nlohmann::json Serialise() const = 0;

    std::shared_ptr<Gene> Copy(unsigned mutationCount) const;

    // MAYBE ExpressCodominantGene(Trilobyte& owner, Phenotype& target, Gene& other)
    virtual void ExpressGene(Trilobyte& owner, Phenotype& target) const = 0;

    // MAYBE Splice (implement a function that will create a new Gene by splicing two parent genes together)
    double GetDominance() const { return dominance_; }

    Gene& operator=(const Gene& other) = delete;

protected:
    constexpr static double BASE_WEIGHT = 100.0;
    const static inline std::string KEY_DOMINANCE = "dominance";

    virtual std::shared_ptr<Gene> Copy() const = 0;

    void Mutate(unsigned mutationCount);
    void AddMutation(const double& probability, std::function<void()>&& action);

private:
    double dominance_;
    /*
     * Add mutations to this in child class constructors. A normal mutation
     * should be given a weight of BASE_WEIGHT
     */
    Random::WeightedContainer<std::function<void()>> mutations_;
};

#endif // GENE_H
