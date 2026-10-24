#pragma once

#include "IntegerChromosome.h"
#include "Interfaces.h"

#include <algorithm>
#include <cstddef>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ga {
using namespace std;

inline bool isCustomerPermutation(const IntegerChromosome& chromosome, size_t customers) {
    if (chromosome.size() != customers)
        return false;
    vector<bool> seen(customers + 1, false);
    for (int gene : chromosome.getGenes()) {
        if (gene < 1 || static_cast<size_t>(gene) > customers || seen[static_cast<size_t>(gene)])
            return false;
        seen[static_cast<size_t>(gene)] = true;
    }
    return true;
}

inline void checkCustomerPermutation(const IntegerChromosome& chromosome, size_t customers, const string& step) {
    if (!isCustomerPermutation(chromosome, customers))
        throw logic_error(step + ": chromosome is not a permutation of customers 1.." + to_string(customers) + ".");
}

inline void checkCustomerPermutations(const Population<IntegerChromosome>& population, size_t customers) {
    for (const IntegerChromosome& chromosome : population)
        checkCustomerPermutation(chromosome, customers, "initial population");
}

inline Population<IntegerChromosome> randomCustomerPermutations(size_t count, size_t customers, mt19937& rng) {
    if (customers == 0)
        throw invalid_argument("randomCustomerPermutations: at least one customer is required.");
    vector<int> order(customers);
    iota(order.begin(), order.end(), 1);

    Population<IntegerChromosome> population;
    population.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        shuffle(order.begin(), order.end(), rng);
        population.emplace_back(order, 1, static_cast<int>(customers));
    }
    checkCustomerPermutations(population, customers);
    return population;
}

class PermutationCheckedCrossover final : public CrossoverStrategy<IntegerChromosome> {
public:
    PermutationCheckedCrossover(shared_ptr<const CrossoverStrategy<IntegerChromosome>> crossover, size_t customers)
        : crossover_(std::move(crossover)), customers_(customers) {
        if (!crossover_)
            throw invalid_argument("PermutationCheckedCrossover: crossover is required.");
    }

    pair<IntegerChromosome, IntegerChromosome> crossover(const IntegerChromosome& parent1,
                                                         const IntegerChromosome& parent2,
                                                         mt19937& rng) const override {
        pair<IntegerChromosome, IntegerChromosome> children = crossover_->crossover(parent1, parent2, rng);
        checkCustomerPermutation(children.first, customers_, "crossover");
        checkCustomerPermutation(children.second, customers_, "crossover");
        return children;
    }

private:
    shared_ptr<const CrossoverStrategy<IntegerChromosome>> crossover_;
    size_t customers_;
};

class PermutationCheckedMutation final : public MutationStrategy<IntegerChromosome> {
public:
    PermutationCheckedMutation(shared_ptr<const MutationStrategy<IntegerChromosome>> mutation, size_t customers)
        : mutation_(std::move(mutation)), customers_(customers) {
        if (!mutation_)
            throw invalid_argument("PermutationCheckedMutation: mutation is required.");
    }

    void mutate(IntegerChromosome& chromosome, double probability, mt19937& rng) const override {
        mutation_->mutate(chromosome, probability, rng);
        checkCustomerPermutation(chromosome, customers_, "mutation");
    }

private:
    shared_ptr<const MutationStrategy<IntegerChromosome>> mutation_;
    size_t customers_;
};

}
