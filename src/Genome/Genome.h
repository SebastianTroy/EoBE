#ifndef GENOME_H
#define GENOME_H

#include "Gene.h"
#include "Phenotype.h"
#include "ChromosomePair.h"
#include "UniverseParameters.h"

#include "fmt/format.h"

#include <vector>
#include <memory>
#include <functional>
#include <string>

class Genome {
public:
    // Create a single Chromosome pair with the following genes
    explicit Genome(std::vector<std::shared_ptr<Gene>>&& genes);
    Genome(std::vector<ChromosomePair>&& chromosomes, uint64_t geneMutationCount, uint64_t chromosomeMutationCount);

    static std::shared_ptr<Genome> CreateOffspring(const Genome& aGenome, const Genome& bGenome, const UniverseParameters& universeParameters);
    static nlohmann::json Serialise(const std::shared_ptr<Genome>& genome);
    // static std::shared_ptr<Genome> Deserialise(const nlohmann::json& serialisedGenome);

    Phenotype GetPhenoType(Trilobyte& owner) const;

    uint64_t GetChromosomeCount() const { return chromosomes_.size(); }
    uint64_t GetGeneCount() const;
    uint64_t GetGeneMutationCount() const { return geneMutationCount_; }
    uint64_t GetChromosomeMutationCount() const { return chromosomeMutationCount_; }

    void Mutate(const UniverseParameters& universeParameters);

private:
    static const inline std::string KEY_GENE_MUTATION_COUNT = "GeneMutations";
    static const inline std::string KEY_CHROMOSOME_MUTATION_COUNT = "ChromosomeMutations";
    static const inline std::string KEY_CHROMOSOMES = "Chromosomes";

    // accumulate thise each generation
    uint64_t geneMutationCount_;
    uint64_t chromosomeMutationCount_;

    std::vector<ChromosomePair> chromosomes_;

    void ForEach(const std::function<void(const Gene& gene)>& action) const;
};

template<>
struct fmt::formatter<Genome>
{
    template<typename ParseContext>
    constexpr auto parse(ParseContext& context)
    {
        return context.begin();
    }

    template <typename FormatContext>
    auto format(const Genome& genome, FormatContext& context)
    {
        return fmt::format_to(context.out(), "{} chromosomes, {} genes, {} mutations", genome.GetChromosomeCount(), genome.GetGeneCount(), genome.GetChromosomeMutationCount() + genome.GetGeneMutationCount());
    }
};

#endif // GENOME_H
