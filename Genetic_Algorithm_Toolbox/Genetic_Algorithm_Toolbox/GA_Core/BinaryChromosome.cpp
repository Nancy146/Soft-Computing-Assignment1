#include "BinaryChromosome.h"

#include <stdexcept>

namespace ga {

using namespace std;

BinaryChromosome::BinaryChromosome(size_t length)
    : genes(length, 0)
{
}

BinaryChromosome::BinaryChromosome(const vector<int>& genes)
    : genes(genes)
{
    if (!isValid()) {
        throw invalid_argument(
            "BinaryChromosome: genes must contain only 0 or 1."
        );
    }
}

void BinaryChromosome::initialize(mt19937& rng)
{
    uniform_int_distribution<int> distribution(0, 1);

    for (auto& gene : genes) {
        gene = distribution(rng);
    }
}

vector<int>& BinaryChromosome::getGenes()
{
    return genes;
}

const vector<int>& BinaryChromosome::getGenes() const
{
    return genes;
}

size_t BinaryChromosome::size() const
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
