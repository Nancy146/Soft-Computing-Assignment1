#pragma once
#include "Interfaces.h"
#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>
using namespace std;

namespace ga {
    template <typename C>
    class OrderCrossover : public CrossoverStrategy<C> {
    public:
        pair<C, C> crossover(const C& parent1, const C& parent2, mt19937& rng) const override {
            size_t n = parent1.getGenes().size();
            if (n != parent2.getGenes().size()) {
                throw invalid_argument("OrderCrossover: parents must have the same length.");
            }
            if (n < 2) {
                return {parent1, parent2};
            }
            uniform_int_distribution<size_t> pick(0, n - 1);
            size_t start = pick(rng);
            size_t end = pick(rng);
            if (start > end) {
                swap(start, end);
            }
            C child1 = makeChild(parent1, parent2, start, end);
            C child2 = makeChild(parent2, parent1, start, end);
            return {child1, child2};
        }
    private:
        C makeChild(const C& keep, const C& other, size_t start, size_t end) const {
            size_t n = keep.getGenes().size();
            C child = keep;
            size_t writePos = (end + 1) % n;
            for (size_t step = 0; step < n; step++) {
                auto gene = other.getGenes()[(end + 1 + step) % n];
                bool inSlice = false;
                for (size_t j = start; j <= end; j++) {
                    if (keep.getGenes()[j] == gene) {
                        inSlice = true;
                        break;
                    }
                }
                if (!inSlice) {
                    child.getGenes()[writePos] = gene;
                    writePos = (writePos + 1) % n;
                }
            }
            return child;
        }
    };
}