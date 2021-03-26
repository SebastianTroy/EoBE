#include "ChromosomePair.h"

#include "GeneFactory.h"

#include <Random.h>
#include <NeuralNetwork.h>

using namespace nlohmann;

ChromosomePair::ChromosomePair(std::vector<std::shared_ptr<Gene> >&& genes)
    : aChromosomeRange_(Random::Number<unsigned>(), Random::Number<unsigned>())
    , bChromosomeRange_(aChromosomeRange_)
{
    for (auto& gene : genes) {
        chromosomePair_.insert({ Random::Number(aChromosomeRange_), std::make_pair(gene, gene) });
    }
}

json ChromosomePair::Serialise(const ChromosomePair& genome)
{
    json alleles = json::array();
    for (const auto& [ locus, genePair ] : genome.chromosomePair_) {
        const auto& [ geneA, geneB ] = genePair;{}
        alleles.push_back({
                              {KEY_ALLELE_LOCUS, locus},
                              {KEY_ALLELE_GENE_A_NAME, geneA ? geneA->GetName() : ""},
                              {KEY_ALLELE_GENE_B_NAME, geneB ? geneB->GetName() : ""},
                              {KEY_ALLELE_GENE_A, geneA ? geneA->Serialise() : json::object()},
                              {KEY_ALLELE_GENE_B, geneB ? geneB->Serialise() : json::object()},
                          });
    }

    return {
        {KEY_A_CHROMOSOME_RANGE, Tril::Range<unsigned>::Serialise(genome.aChromosomeRange_)},
        {KEY_B_CHROMOSOME_RANGE, Tril::Range<unsigned>::Serialise(genome.bChromosomeRange_)},
        {KEY_ALLELES, alleles},
    };
}

std::optional<ChromosomePair> ChromosomePair::Recombine(const ChromosomePair& maternal, const ChromosomePair& paternal)
{
    // Take one chromosome from each pair
    bool takeFirstOfMaternal = Random::Boolean();
    bool takeFirstOfPaternal = Random::Boolean();

    Tril::Range<unsigned> newAChromosomeRange_ = takeFirstOfMaternal ? maternal.aChromosomeRange_ : maternal.bChromosomeRange_;
    Tril::Range<unsigned> newBChromosomeRange_ = takeFirstOfPaternal ? paternal.aChromosomeRange_ : paternal.bChromosomeRange_;

    // TODO consider how this prevents evolution of sexual dimorphism
    // Incompatible if chromosome ranges are too different
    // TODO make the acceptible range a parameter?
    if (newAChromosomeRange_.Similarity(newBChromosomeRange_) < Random::Number(0.95, 0.99)) {
        return {};
    }

    std::map<unsigned, std::pair<std::shared_ptr<Gene>, std::shared_ptr<Gene>>> newPair;
    double unpairedGeneCount = 0;

    // copy genes from one of the maternal chromosomes
    for (auto& [ location, genePair ] : maternal.chromosomePair_) {
        const std::shared_ptr<Gene>& geneToCopy = takeFirstOfMaternal ? genePair.first : genePair.second;
        if (geneToCopy) {
            newPair[location].first = geneToCopy;
            unpairedGeneCount++;
        }
    }
    // copy genes from one of the paternal chromosomes
    for (auto& [ location, genePair ] : paternal.chromosomePair_) {
        const std::shared_ptr<Gene>& geneToCopy = takeFirstOfPaternal ? genePair.first : genePair.second;
        if (geneToCopy) {
            newPair[location].second = geneToCopy;
            if (newPair[location].first != nullptr) {
                --unpairedGeneCount;
            } else {
                ++unpairedGeneCount;
            }
        }
    }

    // TODO consider how this prevents evolution of sexual dimorphism
    // Incompatible if too many unpaired genes
    if (unpairedGeneCount / newPair.size() > Random::Number(0.01, 0.05)) {
        return {};
    }

    /*
     * Crossover Biologically, crossover occurs once sperm and egg have
     * fused, allowing grandchildren of the sperm and egg provider to
     * inherit genes from all four grandparents, despite only recieving one
     * chromosome from each pair from each parent.
     */
    // TODO crossover more likely at tips of chromosomes?
    if (size_t crossoverCount = Random::Poisson<size_t>(1); crossoverCount > 0) {
        std::vector<unsigned> crossoverLocations =  Random::Numbers(crossoverCount, std::min(newAChromosomeRange_.Min(), newBChromosomeRange_.Min()), std::max(newAChromosomeRange_.Max(), newBChromosomeRange_.Max()));
        bool swap = Random::Boolean();
        auto swapLocation = crossoverLocations.cbegin();
        // If started with swap, switch beginning of chromosome ranges
        if (swap) {
            auto aRangeFirst = newAChromosomeRange_.First();
            auto bRangeFirst = newBChromosomeRange_.First();
            newAChromosomeRange_.SetRange(bRangeFirst, newAChromosomeRange_.Last());
            newBChromosomeRange_.SetRange(aRangeFirst, newBChromosomeRange_.Last());
            // If even number of crossover locations we need to the ends of the ranges
            if (crossoverCount % 2 == 0) {
                auto aRangeFirst = newAChromosomeRange_.First();
                auto bRangeFirst = newBChromosomeRange_.First();
                newAChromosomeRange_.SetRange(bRangeFirst, newAChromosomeRange_.Last());
                newBChromosomeRange_.SetRange(aRangeFirst, newBChromosomeRange_.Last());
            }
        } else {
            // If odd number of crossover locations we need to the ends of the ranges
            if (crossoverCount % 2 != 0) {
                auto aRangeFirst = newAChromosomeRange_.First();
                auto bRangeFirst = newBChromosomeRange_.First();
                newAChromosomeRange_.SetRange(bRangeFirst, newAChromosomeRange_.Last());
                newBChromosomeRange_.SetRange(aRangeFirst, newBChromosomeRange_.Last());
            }
        }

        for (auto& [ location, genePair ] : newPair) {
            auto& [ a, b ] = genePair;{}
            while (swapLocation != crossoverLocations.cend() && *swapLocation <= location) {
                ++swapLocation;
                swap = !swap;
            }
            if (swap) {
                a.swap(b);
            }
        }
    }

    return { ChromosomePair(newAChromosomeRange_, newBChromosomeRange_, std::move(newPair)) };
}

void ChromosomePair::ForEach(const std::function<void (const Gene&)>& action) const
{
    for (auto& [ location, pair ] : chromosomePair_) {
        (void) location;
        if (!Gene::Empty(pair)) {
            action(*Gene::GetMostDominant(pair));
        }
    }
}

void ChromosomePair::MutateGene()
{
    if (!chromosomePair_.empty()) {
        // Select random Gene from the pair of chromosomes
        auto& [ position, genePair ] = Random::Item(chromosomePair_);{}
        (void) position; // unused
        // Select a random Gene from the pair
        std::shared_ptr<Gene>& allele = Gene::GetRandom(genePair);
        // A pair may have a missing gene in it, so check for null
        if (allele) {
            allele = allele->Copy(1);
        }
    }
}
#include <iostream>
void ChromosomePair::MutateStructure()
{
    // All equally weighted for now
    auto index = Random::WeightedIndex({ 1, 1, 1, 1, 1, 1 });
    switch (index) {
    case 0:
        // chromosome range change
        for (Tril::Range<unsigned>* range : { &aChromosomeRange_, &bChromosomeRange_ }) {
            unsigned first = range->First() + std::round(Random::Gaussian<double>(0, 50));
            unsigned last = range->Last() + std::round(Random::Gaussian<double>(0, 50));
            range->SetRange(first, last);
        }
        break;
    case 1:
        // gene duplication
        if (!chromosomePair_.empty()) {
            auto& [ location, genePair ] = Random::Item(chromosomePair_);{}
            (void) location;
            std::shared_ptr<Gene>& randomGene = Gene::GetRandom(genePair);
            if (randomGene != nullptr) {
                bool addToA = Random::Boolean();
                unsigned randomLocation = Random::Number<unsigned>(addToA ? aChromosomeRange_ : bChromosomeRange_);
                addToA ? chromosomePair_[randomLocation].first = randomGene : chromosomePair_[randomLocation].second = randomGene;
            }
        }
        break;
    case 2:
        // allele loss
        if (!chromosomePair_.empty()) {
            auto& [ location, genePair ] = Random::Item(chromosomePair_);{}
            (void) location;
            Gene::GetRandom(genePair).reset();
        }
        break;
    case 3:
        // gene loss
        if (!chromosomePair_.empty()) {
            chromosomePair_.erase(Random::Item(chromosomePair_).first);
        }
        break;
    case 4: {
        // gene addition
        /*
             * FIXME perhaps the brain should be a fixed feature of the genome
             * and not a gene?
             *
             * Even if we parsed the genome and extracted the gene width, at
             * this point there'd be no way to tell if the brain width was going
             * to mutate later...
             *
             * Perhaps each gene needs a "BrainWidthMutated(newWidth)" function?
             */
        std::shared_ptr<Gene> randomGene = GeneFactory::Get().GenerateRandomGene(NeuralNetwork::BRAIN_WIDTH);
        bool addToA = Random::Boolean();
        unsigned randomLocation = Random::Number<unsigned>(addToA ? aChromosomeRange_ : bChromosomeRange_);
        addToA ? chromosomePair_[randomLocation].first = randomGene : chromosomePair_[randomLocation].second = randomGene;
    }
        break;
    case 5: {
        if (!chromosomePair_.empty()) {
            // gene transposition
            auto& [ location, genePair ] = Random::Item(chromosomePair_);{}
            (void) location;
            std::shared_ptr<Gene>& randomGene = Gene::GetRandom(genePair);
            if (randomGene) {
                bool addToA = Random::Boolean();
                unsigned randomLocation = Random::Number<unsigned>(addToA ? aChromosomeRange_ : bChromosomeRange_);
                addToA ? chromosomePair_[randomLocation].first = randomGene : chromosomePair_[randomLocation].second = randomGene;
                randomGene.reset();
            }
        }
    }
        break;
        // TODO case 6 : // invert part of a chromosome
    }

    /*
     * Now do some tidying, by removing alleles or gene pairs that are no longer
     * on the chromosomes.
     */

    // Sadly std::remove_if doesn't work for std::map, perhaps future erase_if will...
    for (auto iter = chromosomePair_.begin(); iter != chromosomePair_.end();) {
        const unsigned& location = iter->first;
        auto& [ geneA, geneB ] = iter->second;{}

        /*
         * As each chromosome in the pair may have a slightly different range
         * remove any alleles that fall out of the range of their specific
         * chromosome.
         */
        if (geneA != nullptr && !aChromosomeRange_.Contains(location)) {
            // trim genes outside of their chromosome range
            geneA = nullptr;
        }
        if (geneB != nullptr && !bChromosomeRange_.Contains(location)) {
            // trim genes outside of their chromosome range
            geneB = nullptr;
        }

        bool bothOutOfRange = !aChromosomeRange_.Contains(location) && !bChromosomeRange_.Contains(location);
        bool bothNull = !geneA && !geneB;
        if (bothOutOfRange || bothNull) {
            // Remove any gene pairs that are no longer on either chromosome
            iter = chromosomePair_.erase(iter);
        } else {
            ++iter;
        }
    }
}

ChromosomePair::ChromosomePair(const Tril::Range<unsigned>& aRange, const Tril::Range<unsigned>& bRange, std::map<unsigned, std::pair<std::shared_ptr<Gene>, std::shared_ptr<Gene> > >&& chromosomePair)
    : aChromosomeRange_(aRange)
    , bChromosomeRange_(bRange)
    , chromosomePair_(chromosomePair)
{
}
