#pragma once

#include "Interfaces.h"

#include <cstddef>
#include <random>
#include <stdexcept>

namespace ga {

using namespace std;

template <typename C>
class TournamentSelection : public SelectionStrategy<C> {
private:
    size_t tournamentSize;

public:
    explicit TournamentSelection(size_t tournamentSize = 3)
        : tournamentSize(tournamentSize)
    {
        if (tournamentSize == 0) {
            throw invalid_argument(
                "Tournament size must be greater than zero."
            );
        }
    }

    size_t select(
        const Population<C>& population,
        const vector<double>& fitness,
        mt19937& rng
    ) const override
    {
        if (population.empty()) {
            throw invalid_argument(
                "TournamentSelection: population cannot be empty."
            );
        }

        if (population.size() != fitness.size()) {
            throw invalid_argument(
                "TournamentSelection: population and fitness sizes must match."
            );
        }

        uniform_int_distribution<size_t> distribution(
            0,
            population.size() - 1
        );

        size_t bestIndex = distribution(rng);

        for (size_t i = 1; i < tournamentSize; ++i) {

            size_t candidateIndex = distribution(rng);

            if (fitness[candidateIndex] > fitness[bestIndex]) {
                bestIndex = candidateIndex;
            }
        }

        return bestIndex;
    }
};

}
