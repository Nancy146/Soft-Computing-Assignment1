#pragma once

#include "../Interfaces.h"
#include "../BinaryChromosome.h"

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
        if (probability < 0.0 || probability > 1.0) {
            throw std::invalid_argument(
                "Mutation probability must be between 0 and 1."
            );
        }

        std::uniform_real_distribution<double> distribution(
            0.0,
            1.0
        );

        auto& genes = chromosome.getGenes();

        for (auto& gene : genes) {

            if (distribution(rng) < probability) {
                gene = 1 - gene;
            }
        }
    }
};

}