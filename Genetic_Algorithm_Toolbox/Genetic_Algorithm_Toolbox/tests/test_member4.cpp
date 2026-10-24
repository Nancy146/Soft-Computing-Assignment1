#include "CVRPDecoder.h"
#include "CVRPFeasibility.h"
#include "CVRPPenaltyHandler.h"
#include "GeneticAlgorithm.h"
#include "OrderCrossover.h"
#include "Replacement.h"
#include "SwapMutation.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <vector>

using namespace std;
using namespace ga;

struct BinaryTournament final : SelectionStrategy<IntegerChromosome> {
    size_t select(const Population<IntegerChromosome>& population,
                  const vector<double>& fitness, mt19937& rng) const override {
        uniform_int_distribution<size_t> pick(0, population.size() - 1);
        const size_t a = pick(rng), b = pick(rng);
        return fitness[a] >= fitness[b] ? a : b;
    }
};

static void testGeneticAlgorithmWithIntegerChromosome() {
    CVRPProblem problem({0, 2, 3, 4, 2, 1, 3}, 6, 3);
    const vector<int> identity{1, 2, 3, 4, 5, 6};

    FitnessFunction<IntegerChromosome> fitness = [](const IntegerChromosome& c) {
        double score = 0;
        for (size_t i = 0; i < c.size(); ++i)
            if (c.getGenes()[i] == static_cast<int>(i) + 1) score += 1;
        return score;
    };

    GAConfig config;
    config.populationSize = 20;
    config.maxGenerations = 30;

    mt19937 rng(7);
    Population<IntegerChromosome> population;
    population.emplace_back(identity, 1, 6);
    while (population.size() < config.populationSize) {
        vector<int> genes = identity;
        shuffle(genes.begin(), genes.end(), rng);
        population.emplace_back(genes, 1, 6);
    }

    CVRPFeasibility feasibility(problem);
    GeneticAlgorithm<IntegerChromosome> geneticAlgorithm(
        config, population, fitness, feasibility,
        make_shared<CVRPPenaltyHandler<IntegerChromosome>>(),
        make_shared<BinaryTournament>(),
        make_shared<OrderCrossover<IntegerChromosome>>(),
        make_shared<SwapMutation<IntegerChromosome>>(),
        make_shared<ElitistReplacement<IntegerChromosome>>());
    const GAResult<IntegerChromosome> result = geneticAlgorithm.run();

    assert(result.bestChromosome.getGenes() == identity);
    assert(result.bestFitness == 6.0);
    assert(result.bestGeneration == 0);
    assert(result.bestIsFeasible);
    assert(fitness(result.bestChromosome) == result.bestFitness);
    assert(result.bestHistory.size() == config.maxGenerations + 1);
    assert(*max_element(result.bestHistory.begin(), result.bestHistory.end()) == result.bestFitness);
}

int main() {
    CVRPProblem problem({0, 2, 3, 4, 2}, 5, 3);
    CVRPFeasibility feasible(problem);

    IntegerChromosome good({1, 2, 3, 4}, 1, 4);
    assert(feasible(good));
    const auto routes = CVRPDecoder::decode(good, problem);
    assert(routes.size() == 3);
    assert((routes[0] == vector<int>{1, 2}));
    assert((routes[1] == vector<int>{3}));
    assert((routes[2] == vector<int>{4}));

    IntegerChromosome duplicate({1, 2, 2, 4}, 1, 4);
    assert(!feasible(duplicate));

    CVRPProblem tooFewVehicles({0, 2, 3, 4, 2}, 5, 2);
    CVRPFeasibility tooFewVehiclesCheck(tooFewVehicles);
    assert(!tooFewVehiclesCheck(good));

    CVRPProblem impossibleCustomer({0, 6, 1}, 5, 2);
    CVRPFeasibility impossibleCheck(impossibleCustomer);
    IntegerChromosome impossible({1, 2}, 1, 2);
    assert(!impossibleCheck(impossible));

    CVRPPenaltyHandler<IntegerChromosome> penalty(1e-9);
    FitnessFunction<IntegerChromosome> fitness = [](const IntegerChromosome&) { return 0.5; };
    FeasibilityFunction<IntegerChromosome> feasibility = [](const IntegerChromosome&) { return false; };
    mt19937 rng(42);
    assert(abs(penalty.handle(impossible, fitness, feasibility, rng) - 1e-9) < 1e-15);

    testGeneticAlgorithmWithIntegerChromosome();

    cout << "Member 4 CVRP tests passed.\n";
    return 0;
}
