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
        
        // 0.001 <= Pm <= 0.1
        if (probability < 0.001 || probability > 0.1) {
            throw std::invalid_argument(
                "Mutation probability must be between 0.001 and 0.1."
            );
        }

        std::uniform_real_distribution<double> distribution(
            0.0,
            1.0
        );

        auto& genes = chromosome.getGenes();

        for (auto& gene : genes) {

            double randomNumber = distribution(rng);

            if (randomNumber <= probability) {
                gene = 1 - gene;
            }
        }
    }
};

}
