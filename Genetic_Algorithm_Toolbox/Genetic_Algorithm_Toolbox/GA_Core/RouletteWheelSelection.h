#pragma once

#include "../Interfaces.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace ga {

template <typename C>
class RouletteWheelSelection : public SelectionStrategy<C> {
public:

    std::size_t select(
        const Population<C>& population,
        const std::vector<double>& fitness,
        std::mt19937& rng
    ) const override
    {
        if (population.empty()) {
            throw std::invalid_argument(
                "RouletteWheelSelection: population cannot be empty."
            );
        }

        if (population.size() != fitness.size()) {
            throw std::invalid_argument(
                "RouletteWheelSelection: population and fitness sizes must match."
            );
        }

        double minimumFitness =
            *std::min_element(fitness.begin(), fitness.end());

        std::vector<double> weights(fitness.size());

        double totalWeight = 0.0;

        for (std::size_t i = 0; i < fitness.size(); ++i) {

            weights[i] =
                fitness[i] - minimumFitness;

            totalWeight += weights[i];
        }

        // If all fitness values are equal,
        // every chromosome has the same probability.
        if (totalWeight <= 0.0) {

            std::uniform_int_distribution<std::size_t> distribution(
                0,
                population.size() - 1
            );

            return distribution(rng);
        }

        std::uniform_real_distribution<double> distribution(
            0.0,
            totalWeight
        );

        double randomValue = distribution(rng);

        double cumulative = 0.0;

        for (std::size_t i = 0; i < weights.size(); ++i) {

            cumulative += weights[i];

            if (randomValue <= cumulative) {
                return i;
            }
        }

        // Floating-point safety fallback
        return weights.size() - 1;
    }
};

}