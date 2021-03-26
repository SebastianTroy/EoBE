#include "Genome.h"

#include <Random.h>

using namespace nlohmann;

Genome::Genome(std::vector<std::shared_ptr<Gene> >&& genes)
    : geneMutationCount_(0)
    , chromosomeMutationCount_(0)
    , chromosomes_({ ChromosomePair(std::move(genes)), })

{
}

Genome::Genome(std::vector<ChromosomePair>&& chromosomes, uint64_t geneMutationCount, uint64_t chromosomeMutationCount)
    : geneMutationCount_(geneMutationCount)
    , chromosomeMutationCount_(chromosomeMutationCount)
    , chromosomes_(std::move(chromosomes))
{
}

std::shared_ptr<Genome> Genome::CreateOffspring(const Genome& aGenome, const Genome& bGenome, const UniverseParameters& universeParameters)
{
    if (aGenome.chromosomes_.size() == bGenome.chromosomes_.size()) {
        std::vector<ChromosomePair> newChromosomes;

        auto aIter = aGenome.chromosomes_.cbegin();
        auto bIter = bGenome.chromosomes_.cbegin();
        for (; aIter != aGenome.chromosomes_.cend() ; ++aIter, ++bIter) {
            if (std::optional<ChromosomePair> crossed = ChromosomePair::Recombine(*aIter, *bIter)) {
                newChromosomes.push_back(std::move(crossed.value()));
                continue;
            }
            return {};
        }
        auto offspring = std::make_shared<Genome>(std::move(newChromosomes), (aGenome.GetGeneMutationCount() + bGenome.GetGeneMutationCount()) / 2, (aGenome.GetChromosomeMutationCount() + bGenome.GetChromosomeMutationCount()) / 2);
        offspring->Mutate(universeParameters);
        if (offspring->chromosomes_.size() > 0) {
            return offspring;
        }
    }
    return {};
}

json Genome::Serialise(const std::shared_ptr<Genome>& genome)
{
    if (!genome) {
        return json::object();
    }

    json chromosomes = json::array();

    for (const auto& chromosome : genome->chromosomes_) {
        chromosomes.push_back(ChromosomePair::Serialise(chromosome));
    }

    return {
        {KEY_GENE_MUTATION_COUNT, genome->geneMutationCount_},
        {KEY_CHROMOSOME_MUTATION_COUNT, genome->chromosomeMutationCount_},
        {KEY_CHROMOSOMES, chromosomes},
    };
}

Phenotype Genome::GetPhenoType(Swimmer& owner) const
{
    Phenotype phenotype;
    ForEach([&](const Gene& gene)
    {
        gene.ExpressGene(owner, phenotype);
    });
    if (!phenotype.brain) {
        phenotype.brain = std::make_shared<NeuralNetwork>(1, NeuralNetwork::BRAIN_WIDTH, NeuralNetwork::InitialWeights::PassThrough);
    }
    return phenotype;
}

void Genome::ForEach(const std::function<void (const Gene&)>& action) const {
    for (const ChromosomePair& chromosome : chromosomes_) {
        chromosome.ForEach(action);
    }
}

void Genome::Mutate(const UniverseParameters& universeParameters)
{
    // Floor the count, rounding would bias values > 0
    const unsigned geneMutations = std::max(0.0, Random::Gaussian(universeParameters.meanGeneMutationCount_, universeParameters.geneMutationCountStdDev_));
    const unsigned structuralMutations = std::max(0.0, Random::Gaussian(universeParameters.meanStructuralMutationCount_, universeParameters.structuralMutationCountStdDev_));

    geneMutationCount_ += geneMutations;
    chromosomeMutationCount_ += structuralMutations;

    for (unsigned i = 0; i < structuralMutations; ++i) {
        switch (Random::WeightedIndex({ 0.9, 0.04, 0.04, 0.002 })) {
        case 0:
            if (!chromosomes_.empty()) {
                Random::Item(chromosomes_).MutateStructure();
            }
            break;
        case 1:
            // duplicate a chromosome
            if (!chromosomes_.empty()) {
                ChromosomePair copy = Random::Item(chromosomes_);
                chromosomes_.emplace_back(std::move(copy));
            }
            break;
        case 2:
            // delete a chromosome
            if (!chromosomes_.empty()) {
                int randomChromosome = Random::Number<int>(0, static_cast<int>(chromosomes_.size()) - 1);
                chromosomes_.erase(chromosomes_.begin() + Random::Number<int>(0, randomChromosome));
            }
            break;
        case 3:
            // duplicate all chromosomes
            std::vector<ChromosomePair> copy = chromosomes_;
            std::move(std::begin(copy), std::end(copy), std::back_inserter(chromosomes_));
            break;
            // TODO case 4: chromosome fusing
            // TODO case 5: chromosome fragmenting
        }
    }

    if (!chromosomes_.empty()) {
        for (unsigned i = 0; i < geneMutations; ++i) {
            Random::Item(chromosomes_).MutateGene();
        }
    }
}

uint64_t Genome::GetGeneCount() const
{
    uint64_t count = 0;
    ForEach([&](const auto& /*gene*/)
    {
        ++count;
    });
    return count;
}
