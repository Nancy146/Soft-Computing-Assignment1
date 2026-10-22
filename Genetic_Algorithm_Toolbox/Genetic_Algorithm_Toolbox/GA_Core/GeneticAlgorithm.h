#pragma once
#include "GAConfig.h"
#include "Interfaces.h"
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ga {

using namespace std;

template <typename C>
struct GAResult {
    C bestChromosome;
    double bestFitness = 0.0;
    size_t bestGeneration = 0;
    bool bestIsFeasible = false;

    vector<double> bestHistory;     // best fitness in the population
    vector<double> averageHistory;  // average fitness of the population
};

template <typename C>
class GeneticAlgorithm {
public:
    GeneticAlgorithm(GAConfig config,
                     Population<C> initialPopulation,
                     FitnessFunction<C> fitnessFunction,
                     FeasibilityFunction<C> feasibilityFunction,
                     shared_ptr<const InfeasibilityHandler<C>> infeasibilityHandler,
                     shared_ptr<const SelectionStrategy<C>> selection,
                     shared_ptr<const CrossoverStrategy<C>> crossover,
                     shared_ptr<const MutationStrategy<C>> mutation,
                     shared_ptr<const ReplacementStrategy<C>> replacement)
        : config_(move(config)),
          initialPopulation_(move(initialPopulation)),
          fitnessFunction_(move(fitnessFunction)),
          feasibilityFunction_(move(feasibilityFunction)),
          infeasibilityHandler_(move(infeasibilityHandler)),
          selection_(move(selection)),
          crossover_(move(crossover)),
          mutation_(move(mutation)),
          replacement_(move(replacement)) {
        validate();
    }

    GAResult<C> run() const {
        mt19937 rng(config_.seed);
        const size_t n = config_.populationSize;

        Population<C> population = initialPopulation_;
        vector<double> fitness;
        fitness.reserve(n);
        for (C& chromosome : population)
            fitness.push_back(evaluate(chromosome, rng));

        GAResult<C> result;
        result.bestHistory.reserve(config_.maxGenerations + 1);
        result.averageHistory.reserve(config_.maxGenerations + 1);

        size_t best = bestIndex(fitness);
        result.bestChromosome = population[best];
        result.bestFitness = fitness[best];
        result.bestGeneration = 0;
        record(fitness, best, result);

        uniform_real_distribution<double> unit(0.0, 1.0);
        Population<C> offspring;
        vector<double> offspringFitness;

        for (size_t generation = 1; generation <= config_.maxGenerations; ++generation) {
            offspring.clear();
            offspringFitness.clear();
            offspring.reserve(n);
            offspringFitness.reserve(n);

            while (offspring.size() < n) {
                // Selection
                const C& parent1 = population[checkedIndex(selection_->select(population, fitness, rng), n)];
                const C& parent2 = population[checkedIndex(selection_->select(population, fitness, rng), n)];

                // Crossover with probability Pc, otherwise the children are copies of the parents.
                pair<C, C> children = (unit(rng) < config_.crossoverProbability)
                                          ? crossover_->crossover(parent1, parent2, rng)
                                          : make_pair(parent1, parent2);

                // Mutation + evaluation of the offspring.
                addChild(move(children.first), offspring, offspringFitness, rng);
                if (offspring.size() < n)
                    addChild(move(children.second), offspring, offspringFitness, rng);
            }

            ReplacementResult<C> next =
                replacement_->replace(population, fitness, offspring, offspringFitness);
            if (next.population.size() != n || next.fitness.size() != n)
                throw logic_error("Replacement must return populationSize chromosomes and fitness values.");

            population = move(next.population);
            fitness = move(next.fitness);

            best = bestIndex(fitness);
            if (fitness[best] > result.bestFitness) {
                result.bestChromosome = population[best];
                result.bestFitness = fitness[best];
                result.bestGeneration = generation;
            }
            record(fitness, best, result);
        }

        result.bestIsFeasible = feasibilityFunction_(result.bestChromosome);
        return result;
    }

    const GAConfig& config() const { return config_; }

private:
    void validate() const {
        config_.validate();

        if (initialPopulation_.size() != config_.populationSize)
            throw invalid_argument("Initial population size must equal config.populationSize.");
        if (!fitnessFunction_)
            throw invalid_argument("Fitness function is required.");
        if (!feasibilityFunction_)
            throw invalid_argument("Feasibility function is required.");
        if (!infeasibilityHandler_)
            throw invalid_argument("Infeasibility handler is required.");
        if (!selection_ || !crossover_ || !mutation_ || !replacement_)
            throw invalid_argument("Selection, crossover, mutation and replacement strategies are required.");
    }

    double evaluate(C& chromosome, mt19937& rng) const {
        if (feasibilityFunction_(chromosome))
            return fitnessFunction_(chromosome);
        return infeasibilityHandler_->handle(chromosome, fitnessFunction_, feasibilityFunction_, rng);
    }

    void addChild(C child, Population<C>& offspring, vector<double>& offspringFitness, mt19937& rng) const {
        mutation_->mutate(child, config_.mutationProbability, rng);
        offspringFitness.push_back(evaluate(child, rng));
        offspring.push_back(move(child));
    }

    static size_t bestIndex(const vector<double>& fitness) {
        return static_cast<size_t>(distance(fitness.begin(), max_element(fitness.begin(), fitness.end())));
    }

    static size_t checkedIndex(size_t index, size_t populationSize) {
        if (index >= populationSize)
            throw out_of_range("Selection returned an index outside the population.");
        return index;
    }

    static void record(const vector<double>& fitness, size_t best, GAResult<C>& result) {
        result.bestHistory.push_back(fitness[best]);
        result.averageHistory.push_back(
            accumulate(fitness.begin(), fitness.end(), 0.0) / static_cast<double>(fitness.size()));
    }

    GAConfig config_;
    Population<C> initialPopulation_;

    FitnessFunction<C> fitnessFunction_;
    FeasibilityFunction<C> feasibilityFunction_;
    shared_ptr<const InfeasibilityHandler<C>> infeasibilityHandler_;

    shared_ptr<const SelectionStrategy<C>> selection_;
    shared_ptr<const CrossoverStrategy<C>> crossover_;
    shared_ptr<const MutationStrategy<C>> mutation_;
    shared_ptr<const ReplacementStrategy<C>> replacement_;
};

}
