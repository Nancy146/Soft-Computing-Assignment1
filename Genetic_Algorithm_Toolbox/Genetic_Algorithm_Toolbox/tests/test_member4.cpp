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

using ga::CVRPDecoder;
using ga::CVRPFeasibility;
using ga::CVRPProblem;
using ga::IntegerChromosome;

struct BinaryTournament final : ga::SelectionStrategy<IntegerChromosome> {
    std::size_t select(const ga::Population<IntegerChromosome>& population,
                       const std::vector<double>& fitness, std::mt19937& rng) const override {
        std::uniform_int_distribution<std::size_t> pick(0, population.size() - 1);
        const std::size_t a = pick(rng), b = pick(rng);
        return fitness[a] >= fitness[b] ? a : b;
    }
};

static void testGeneticAlgorithmWithIntegerChromosome() {
    CVRPProblem problem({0, 2, 3, 4, 2, 1, 3}, 6, 3);
    const std::vector<int> identity{1, 2, 3, 4, 5, 6};

    ga::FitnessFunction<IntegerChromosome> fitness = [](const IntegerChromosome& c) {
        double score = 0;
        for (std::size_t i = 0; i < c.size(); ++i)
            if (c.getGenes()[i] == static_cast<int>(i) + 1) score += 1;
        return score;
    };

    ga::GAConfig config;
    config.populationSize = 20;
    config.maxGenerations = 30;

    std::mt19937 rng(7);
    ga::Population<IntegerChromosome> population;
    population.emplace_back(identity, 1, 6);
    while (population.size() < config.populationSize) {
        std::vector<int> genes = identity;
        std::shuffle(genes.begin(), genes.end(), rng);
        population.emplace_back(genes, 1, 6);
    }

    CVRPFeasibility feasibility(problem);
    ga::GeneticAlgorithm<IntegerChromosome> geneticAlgorithm(
        config, population, fitness, feasibility,
        std::make_shared<ga::CVRPPenaltyHandler<IntegerChromosome>>(),
        std::make_shared<BinaryTournament>(),
        std::make_shared<ga::OrderCrossover<IntegerChromosome>>(),
        std::make_shared<ga::SwapMutation<IntegerChromosome>>(),
        std::make_shared<ga::ElitistReplacement<IntegerChromosome>>());
    const ga::GAResult<IntegerChromosome> result = geneticAlgorithm.run();

    assert(result.bestChromosome.getGenes() == identity);
    assert(result.bestFitness == 6.0);
    assert(result.bestGeneration == 0);
    assert(result.bestIsFeasible);
    assert(fitness(result.bestChromosome) == result.bestFitness);
    assert(result.bestHistory.size() == config.maxGenerations + 1);
    assert(*std::max_element(result.bestHistory.begin(), result.bestHistory.end()) == result.bestFitness);
}

int main() {
    CVRPProblem problem({0, 2, 3, 4, 2}, 5, 3);
    CVRPFeasibility feasible(problem);

    IntegerChromosome good({1, 2, 3, 4}, 1, 4);
    assert(feasible(good));
    const auto routes = CVRPDecoder::decode(good, problem);
    assert(routes.size() == 3);
    assert((routes[0] == std::vector<int>{1, 2}));
    assert((routes[1] == std::vector<int>{3}));
    assert((routes[2] == std::vector<int>{4}));

    IntegerChromosome duplicate({1, 2, 2, 4}, 1, 4);
    assert(!feasible(duplicate));

    CVRPProblem tooFewVehicles({0, 2, 3, 4, 2}, 5, 2);
    CVRPFeasibility tooFewVehiclesCheck(tooFewVehicles);
    assert(!tooFewVehiclesCheck(good));

    CVRPProblem impossibleCustomer({0, 6, 1}, 5, 2);
    CVRPFeasibility impossibleCheck(impossibleCustomer);
    IntegerChromosome impossible({1, 2}, 1, 2);
    assert(!impossibleCheck(impossible));

    ga::CVRPPenaltyHandler<IntegerChromosome> penalty(1e-9);
    ga::FitnessFunction<IntegerChromosome> fitness = [](const IntegerChromosome&) { return 0.5; };
    ga::FeasibilityFunction<IntegerChromosome> feasibility = [](const IntegerChromosome&) { return false; };
    std::mt19937 rng(42);
    assert(std::abs(penalty.handle(impossible, fitness, feasibility, rng) - 1e-9) < 1e-15);

    testGeneticAlgorithmWithIntegerChromosome();

    std::cout << "Member 4 CVRP tests passed.\n";
    return 0;
}
