#pragma once
#include "Interfaces.h"
#include <algorithm>
#include <random>
#include <utility>
using namespace std;

namespace ga {
    template <typename C>
    class InversionMutation : public MutationStrategy<C> {
    public:
        void mutate(C& chromosome, double probability, mt19937& rng) const override {
            checkMutationProbability(probability);
            size_t n = chromosome.getGenes().size();
            if (n < 2) {
                return;
            }
            if (randomUnitInclusive(rng) > probability) {
                return;
            }
            uniform_int_distribution<size_t> pick(0, n - 1);
            size_t start = pick(rng);
            size_t end = pick(rng);
            if (start > end) {
                swap(start, end);
            }
            reverse(chromosome.getGenes().begin() + start, chromosome.getGenes().begin() + end + 1);
        }
    };
}
