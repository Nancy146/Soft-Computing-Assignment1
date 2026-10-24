#pragma once

#include "Interfaces.h"

#include <cstddef>
#include <random>
#include <stdexcept>

namespace ga {

template <typename C>
class TournamentSelection : public SelectionStrategy<C> {
private:
    std::size_t tournamentSize;

public:
    explicit TournamentSelection(std::size_t tournamentSize = 3)
        : tournamentSize(tournamentSize)
    {
        if (tournamentSize == 0) {
            throw std::invalid_argument(
                "Tournament size must be greater than zero."
            );
        }
    }

    std::size_t select(
        const Population<C>& population,
        const std::vector<double>& fitness,
        std::mt19937& rng
    ) const override
    {
        if (population.empty()) {
            throw std::invalid_argument(
                "TournamentSelection: population cannot be empty."
            );
        }

        if (population.size() != fitness.size()) {
            throw std::invalid_argument(
                "TournamentSelection: population and fitness sizes must match."
            );
        }

        std::uniform_int_distribution<std::size_t> distribution(
            0,
            population.size() - 1
        );

        std::size_t bestIndex = distribution(rng);

        for (std::size_t i = 1; i < tournamentSize; ++i) {

            std::size_t candidateIndex = distribution(rng);

            if (fitness[candidateIndex] > fitness[bestIndex]) {
                bestIndex = candidateIndex;
            }
        }

        return bestIndex;
    }
};

}