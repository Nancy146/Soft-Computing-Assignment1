#pragma once
#include <cstddef>
#include <cstdint>
#include <functional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ga {
using namespace std;

template <typename C>
using Population = vector<C>;

inline double randomUnitInclusive(mt19937& rng) {
    constexpr uint64_t steps = uint64_t{1} << 53;
    return static_cast<double>(uniform_int_distribution<uint64_t>(0, steps)(rng)) /
           static_cast<double>(steps);
}

inline void checkMutationProbability(double probability) {
    if (!(probability >= 0.001 && probability <= 0.1))
        throw invalid_argument("Mutation probability must be in [0.001, 0.1] (inclusive).");
}

template <typename C>
using FitnessFunction = function<double(const C&)>;

template <typename C>
using FeasibilityFunction = function<bool(const C&)>;

template <typename C>
class SelectionStrategy {
public:
    virtual ~SelectionStrategy() = default;
    virtual size_t select(const Population<C>& population,
                          const vector<double>& fitness,
                          mt19937& rng) const = 0;
};

template <typename C>
class CrossoverStrategy {
public:
    virtual ~CrossoverStrategy() = default;
    virtual pair<C, C> crossover(const C& parent1,
                                 const C& parent2,
                                 mt19937& rng) const = 0;
};

template <typename C>
class MutationStrategy {
public:
    virtual ~MutationStrategy() = default;
    virtual void mutate(C& chromosome,
                        double probability,
                        mt19937& rng) const = 0;
};

template <typename C>
class InfeasibilityHandler {
public:
    virtual ~InfeasibilityHandler() = default;
    virtual double handle(C& chromosome,
                          const FitnessFunction<C>& fitness,
                          const FeasibilityFunction<C>& feasibility,
                          mt19937& rng) const = 0;
};

template <typename C>
struct ReplacementResult {
    Population<C> population;
    vector<double> fitness;
};

template <typename C>
class ReplacementStrategy {
public:
    virtual ~ReplacementStrategy() = default;
    virtual ReplacementResult<C> replace(const Population<C>& currentPopulation,
                                         const vector<double>& currentFitness,
                                         const Population<C>& offspring,
                                         const vector<double>& offspringFitness) const = 0;
};

}
