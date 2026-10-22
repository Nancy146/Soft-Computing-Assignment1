#pragma once
#include "Interfaces.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace ga {

using namespace std;

template <typename C>
class GenerationalReplacement final : public ReplacementStrategy<C> {
public:
    ReplacementResult<C> replace(const Population<C>& currentPopulation,
                                 const vector<double>& /*currentFitness*/,
                                 const Population<C>& offspring,
                                 const vector<double>& offspringFitness) const override {
        if (offspring.size() != currentPopulation.size())
            throw invalid_argument("GenerationalReplacement: offspring count must equal population size.");

        return {offspring, offspringFitness};
    }
};

template <typename C>
class ElitistReplacement final : public ReplacementStrategy<C> {
public:
    ReplacementResult<C> replace(const Population<C>& currentPopulation,
                                 const vector<double>& currentFitness,
                                 const Population<C>& offspring,
                                 const vector<double>& offspringFitness) const override {
        const size_t populationSize = currentPopulation.size();
        if (populationSize == 0)
            throw invalid_argument("ElitistReplacement: current population is empty.");
        if (offspring.size() + 1 < populationSize)
            throw invalid_argument("ElitistReplacement: not enough offspring to fill the population.");

        ReplacementResult<C> next;
        next.population.reserve(populationSize);
        next.fitness.reserve(populationSize);

        // 1) Best individual of the current population.
        const size_t elite = static_cast<size_t>(distance(
            currentFitness.begin(), max_element(currentFitness.begin(), currentFitness.end())));
        next.population.push_back(currentPopulation[elite]);
        next.fitness.push_back(currentFitness[elite]);

        // 2) Best (populationSize - 1) offspring. Only the needed part is sorted.
        const size_t remaining = populationSize - 1;
        vector<size_t> order(offspring.size());
        iota(order.begin(), order.end(), size_t{0});
        partial_sort(order.begin(), order.begin() + remaining, order.end(),
                     [&offspringFitness](size_t a, size_t b) {
                         return offspringFitness[a] > offspringFitness[b];
                     });

        for (size_t i = 0; i < remaining; ++i) {
            next.population.push_back(offspring[order[i]]);
            next.fitness.push_back(offspringFitness[order[i]]);
        }
        return next;
    }
};

}
