#pragma once
#include "Interfaces.h"
#include <random>
#include <utility>
using namespace std;

namespace ga {
    template <typename C>
    class SwapMutation : public MutationStrategy<C> {
    public:
        void mutate(C& chromosome, double probability, mt19937& rng) const override {
            checkMutationProbability(probability);
            size_t n = chromosome.getGenes().size();
            if (n < 2) {
                return;
            }
            uniform_int_distribution<size_t> pick(0, n - 1);
            for (size_t i = 0; i < n; i++) {
                if (randomUnitInclusive(rng) <= probability) {
                    size_t j = pick(rng);
                    swap(chromosome.getGenes()[i], chromosome.getGenes()[j]);
                }
            }
        }
    };
}