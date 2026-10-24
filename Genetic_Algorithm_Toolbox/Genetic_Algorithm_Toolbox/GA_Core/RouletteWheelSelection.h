#pragma once

#include "Interfaces.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace ga {

using namespace std;

template <typename C>
class RouletteWheelSelection : public SelectionStrategy<C> {
public:

    size_t select(
        const Population<C>& population,
        const vector<double>& fitness,
        mt19937& rng
    ) const override
    {
        if (population.empty()) {
            throw invalid_argument(
                "RouletteWheelSelection: population cannot be empty."
            );
        }

        if (population.size() != fitness.size()) {
            throw invalid_argument(
                "RouletteWheelSelection: population and fitness sizes must match."
            );
        }

        double minimumFitness =
            *min_element(fitness.begin(), fitness.end());

        vector<double> weights(fitness.size());

        double totalWeight = 0.0;

        for (size_t i = 0; i < fitness.size(); ++i) {
            weights[i] =
                fitness[i] - minimumFitness;

            totalWeight += weights[i];
        }

        if (totalWeight <= 0.0) {
            uniform_int_distribution<size_t> distribution(
                0,
                population.size() - 1
            );

            return distribution(rng);
        }

        uniform_real_distribution<double> distribution(
            0.0,
            totalWeight
        );

        double randomValue = distribution(rng);

        double cumulative = 0.0;

        for (size_t i = 0; i < weights.size(); ++i) {
            cumulative += weights[i];

            if (randomValue <= cumulative) {
                return i;
            }
        }

        return weights.size() - 1;
    }
};

}
