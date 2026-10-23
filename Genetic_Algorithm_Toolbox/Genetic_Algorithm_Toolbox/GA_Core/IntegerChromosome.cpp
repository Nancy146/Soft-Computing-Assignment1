#include "IntegerChromosome.h"

#include <stdexcept>

namespace ga {

IntegerChromosome::IntegerChromosome(
    std::size_t length,
    int minValue,
    int maxValue
)
    : genes(length),
      minValue(minValue),
      maxValue(maxValue)
{
    if (minValue > maxValue) {
        throw std::invalid_argument(
            "IntegerChromosome: minValue cannot be greater than maxValue."
        );
    }
}

IntegerChromosome::IntegerChromosome(
    const std::vector<int>& genes,
    int minValue,
    int maxValue
)
    : genes(genes),
      minValue(minValue),
      maxValue(maxValue)
{
    if (minValue > maxValue) {
        throw std::invalid_argument(
            "IntegerChromosome: minValue cannot be greater than maxValue."
        );
    }

    if (!isValid()) {
        throw std::invalid_argument(
            "IntegerChromosome: gene is outside the allowed range."
        );
    }
}

void IntegerChromosome::initialize(std::mt19937& rng)
{
    std::uniform_int_distribution<int> distribution(
        minValue,
        maxValue
    );

    for (auto& gene : genes) {
        gene = distribution(rng);
    }
}

std::vector<int>& IntegerChromosome::getGenes()
{
    return genes;
}

const std::vector<int>& IntegerChromosome::getGenes() const
{
    return genes;
}

std::size_t IntegerChromosome::size() const
{
    return genes.size();
}

bool IntegerChromosome::isValid() const
{
    for (int gene : genes) {
        if (gene < minValue || gene > maxValue) {
            return false;
        }
    }

    return true;
}

int IntegerChromosome::getMinValue() const
{
    return minValue;
}

int IntegerChromosome::getMaxValue() const
{
    return maxValue;
}

}