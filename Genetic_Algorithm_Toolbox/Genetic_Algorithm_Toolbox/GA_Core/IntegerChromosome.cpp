#include "IntegerChromosome.h"

#include <stdexcept>

namespace ga {

using namespace std;

IntegerChromosome::IntegerChromosome(
    size_t length,
    int minValue,
    int maxValue
)
    : genes(length),
      minValue(minValue),
      maxValue(maxValue)
{
    if (minValue > maxValue) {
        throw invalid_argument(
            "IntegerChromosome: minValue cannot be greater than maxValue."
        );
    }
}

IntegerChromosome::IntegerChromosome(
    const vector<int>& genes,
    int minValue,
    int maxValue
)
    : genes(genes),
      minValue(minValue),
      maxValue(maxValue)
{
    if (minValue > maxValue) {
        throw invalid_argument(
            "IntegerChromosome: minValue cannot be greater than maxValue."
        );
    }

    if (!isValid()) {
        throw invalid_argument(
            "IntegerChromosome: gene is outside the allowed range."
        );
    }
}

void IntegerChromosome::initialize(mt19937& rng)
{
    uniform_int_distribution<int> distribution(
        minValue,
        maxValue
    );

    for (auto& gene : genes) {
        gene = distribution(rng);
    }
}

vector<int>& IntegerChromosome::getGenes()
{
    return genes;
}

const vector<int>& IntegerChromosome::getGenes() const
{
    return genes;
}

size_t IntegerChromosome::size() const
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
