#pragma once
#include "Interfaces.h"
#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>
using namespace std;

namespace ga {
    template <typename C>
    class MultiPointCrossover : public CrossoverStrategy<C> {
    public:
        MultiPointCrossover(size_t numPoints = 2) {
            if (numPoints == 0) {
                throw invalid_argument("MultiPointCrossover: number of points must be at least 1.");
            }
            k_ = numPoints;
        }
        pair<C, C> crossover(const C& parent1, const C& parent2, mt19937& rng) const override {
            size_t n = parent1.getGenes().size();
            if (n != parent2.getGenes().size()) {
                throw invalid_argument("MultiPointCrossover: parents must have the same length.");
            }
            if (n < 2) {
                return {parent1, parent2};
            }
            C child1 = parent1;
            C child2 = parent2;

            vector<size_t> cuts(n - 1);
            iota(cuts.begin(), cuts.end(), 1);
            shuffle(cuts.begin(), cuts.end(), rng);
            if (k_ < n - 1) {
                cuts.resize(k_);
            }
            sort(cuts.begin(), cuts.end());
            bool swapping = false;
            size_t nextCut = 0;
            for (size_t i = 0; i < n; i++) {
                if (nextCut < cuts.size() && i == cuts[nextCut]) {
                    swapping = !swapping;
                    nextCut++;
                }
                if (swapping) {
                    swap(child1.getGenes()[i], child2.getGenes()[i]);
                }
            }
            return {child1, child2};
        }
    private:
        size_t k_;
    };
}