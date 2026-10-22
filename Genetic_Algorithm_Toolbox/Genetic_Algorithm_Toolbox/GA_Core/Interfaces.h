#pragma once
#include <cstddef>
#include <functional>
#include <random>
#include <utility>
#include <vector>


namespace ga {

using namespace std;

template <typename C>
using Population = vector<C>;

// Provided by the user of the toolbox (CVRP: Member 5).
template <typename C>
using FitnessFunction = function<double(const C&)>;

// Provided by the user of the toolbox (CVRP: Member 4).
template <typename C>
using FeasibilityFunction = function<bool(const C&)>;

// Member 2. Returns the INDEX of the selected parent in `population`.
template <typename C>
class SelectionStrategy {
public:
    virtual ~SelectionStrategy() = default;
    virtual size_t select(const Population<C>& population,
                          const vector<double>& fitness,
                          mt19937& rng) const = 0;
};

// Member 3. Always performs crossover; the GA loop has already decided
// (with probability Pc) that crossover should happen. Parents are not modified.
template <typename C>
class CrossoverStrategy {
public:
    virtual ~CrossoverStrategy() = default;
    virtual pair<C, C> crossover(const C& parent1,
                                 const C& parent2,
                                 mt19937& rng) const = 0;
};

// Members 2 & 3. Mutates `chromosome` in place.
// `probability` is GAConfig::mutationProbability (Pm), unchanged.
template <typename C>
class MutationStrategy {
public:
    virtual ~MutationStrategy() = default;
    virtual void mutate(C& chromosome,
                        double probability,
                        mt19937& rng) const = 0;
};

// Member 4. Called by the GA ONLY for chromosomes that fail the feasibility
// check. Decides what happens to them (penalty, repair, ...) and returns the
// fitness to use. A repair strategy may modify `chromosome` in place.
template <typename C>
class InfeasibilityHandler {
public:
    virtual ~InfeasibilityHandler() = default;
    virtual double handle(C& chromosome,
                          const FitnessFunction<C>& fitness,
                          const FeasibilityFunction<C>& feasibility,
                          mt19937& rng) const = 0;
};

// Returned by replacement so the GA keeps survivors' fitness instead of
// evaluating them again. fitness[i] belongs to population[i].
template <typename C>
struct ReplacementResult {
    Population<C> population;
    vector<double> fitness;
};

//Member1
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
