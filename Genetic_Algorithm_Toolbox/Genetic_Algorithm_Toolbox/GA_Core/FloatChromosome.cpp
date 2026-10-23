#include "FloatChromosome.h"

#include <stdexcept>

namespace ga {

FloatChromosome::FloatChromosome(
    std::size_t length,
    double minValue,
    double maxValue
)
    : genes(length),
      minValue(minValue),
      maxValue(maxValue)
{
    if (minValue > maxValue) {
        throw std::invalid_argument(
            "FloatChromosome: minValue cannot be greater than maxValue."
        );
    }
}

FloatChromosome::FloatChromosome(
    const std::vector<double>& genes,
    double minValue,
    double maxValue
)
    : genes(genes),
      minValue(minValue),
      maxValue(maxValue)
{
    if (minValue > maxValue) {
        throw std::invalid_argument(
            "FloatChromosome: minValue cannot be greater than maxValue."
        );
    }

    if (!isValid()) {
        throw std::invalid_argument(
            "FloatChromosome: gene is outside the allowed range."
        );
    }
}

void FloatChromosome::initialize(std::mt19937& rng)
{
    std::uniform_real_distribution<double> distribution(
        minValue,
        maxValue
    );

    for (auto& gene : genes) {
        gene = distribution(rng);
    }
}

std::vector<double>& FloatChromosome::getGenes()
{
    return genes;
}

const std::vector<double>& FloatChromosome::getGenes() const
{
    return genes;
}

std::size_t FloatChromosome::size() const
{
    return genes.size();
}

bool FloatChromosome::isValid() const
{
    for (double gene : genes) {
        if (gene < minValue || gene > maxValue) {
            return false;
        }
    }

    return true;
}

double FloatChromosome::getMinValue() const
{
    return minValue;
}

double FloatChromosome::getMaxValue() const
{
    return maxValue;
}

}