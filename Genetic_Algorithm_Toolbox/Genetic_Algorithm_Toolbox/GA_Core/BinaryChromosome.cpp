#include "BinaryChromosome.h"

#include <stdexcept>

namespace ga {

BinaryChromosome::BinaryChromosome(std::size_t length)
    : genes(length, 0)
{
}

BinaryChromosome::BinaryChromosome(const std::vector<int>& genes)
    : genes(genes)
{
    if (!isValid()) {
        throw std::invalid_argument(
            "BinaryChromosome: genes must contain only 0 or 1."
        );
    }
}

void BinaryChromosome::initialize(std::mt19937& rng)
{
    std::uniform_int_distribution<int> distribution(0, 1);

    for (auto& gene : genes) {
        gene = distribution(rng);
    }
}

std::vector<int>& BinaryChromosome::getGenes()
{
    return genes;
}

const std::vector<int>& BinaryChromosome::getGenes() const
{
    return genes;
}

std::size_t BinaryChromosome::size() const
{
    return genes.size();
}

bool BinaryChromosome::isValid() const
{
    for (int gene : genes) {
        if (gene != 0 && gene != 1) {
            return false;
        }
    }

    return true;
}

}