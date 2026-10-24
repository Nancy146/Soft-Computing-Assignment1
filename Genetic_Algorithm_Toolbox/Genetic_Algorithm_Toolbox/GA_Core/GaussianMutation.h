#pragma once
#include "Interfaces.h"
#include <random>
#include <stdexcept>
using namespace std;

namespace ga {
    template <typename C>
    class GaussianMutation : public MutationStrategy<C> {
    public:
        GaussianMutation(double lower, double upper, double sigma = 0.1) {
            if (lower > upper) {
                throw invalid_argument("GaussianMutation: lower bound must be <= upper bound.");
            }
            if (sigma <= 0.0) {
                throw invalid_argument("GaussianMutation: sigma must be positive.");
            }
            lower_ = lower;
            upper_ = upper;
            sigma_ = sigma;
        }
        void mutate(C& chromosome, double probability, mt19937& rng) const override {
            checkMutationProbability(probability);
            normal_distribution<double> noise(0.0, sigma_);
            for (auto& gene : chromosome.getGenes()) {
                if (randomUnitInclusive(rng) <= probability) {
                    double newValue = gene + noise(rng);
                    if (newValue < lower_) {
                        newValue = lower_;
                    }
                    if (newValue > upper_) {
                        newValue = upper_;
                    }
                    gene = newValue;
                }
            }
        }
    private:
        double lower_;
        double upper_;
        double sigma_;
    };
}