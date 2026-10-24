#pragma once

#include "Interfaces.h"
#include "BinaryChromosome.h"

#include <random>
#include <stdexcept>

namespace ga {
class BitFlipMutation
    : public MutationStrategy<BinaryChromosome>
{
public:

    void mutate(
        BinaryChromosome& chromosome,
        double probability,
        std::mt19937& rng
    ) const override
    {
        checkMutationProbability(probability);

        auto& genes = chromosome.getGenes();

        for (auto& gene : genes) {
            if (randomUnitInclusive(rng) <= probability) {
                gene = 1 - gene;
            }
        }
    }
};

}
