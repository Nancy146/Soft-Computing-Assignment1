#pragma once

#include "CVRPEvaluator.h"
#include "Interfaces.h"

#include <stdexcept>
#include <utility>
using namespace std;
namespace ga {
class CVRPFeasibleFirstHandler final : public InfeasibilityHandler<IntegerChromosome> {
public:
    explicit CVRPFeasibleFirstHandler(CVRPEvaluator evaluator) : evaluator_(std::move(evaluator)) {}

    double handle(IntegerChromosome& chromosome, const FitnessFunction<IntegerChromosome>&,
                  const FeasibilityFunction<IntegerChromosome>&, mt19937&) const override
    {
        return evaluator_.fitness(chromosome);
    }

private:
    CVRPEvaluator evaluator_;
};

template <typename C>
class CVRPPenaltyHandler final : public InfeasibilityHandler<C> {
public:
    explicit CVRPPenaltyHandler(double infeasibleFitness = 1e-12): infeasibleFitness_(infeasibleFitness) {
        if (infeasibleFitness_ < 0.0)
            throw invalid_argument("Infeasible fitness must be non-negative.");
    }
    double handle(C& chromosome, const FitnessFunction<C>& fitness, const FeasibilityFunction<C>& feasibility, mt19937& rng) const override
    {
        (void)chromosome;
        (void)fitness;
        (void)feasibility;
        (void)rng;
        return infeasibleFitness_;
    }
private:
    double infeasibleFitness_;
};

}
