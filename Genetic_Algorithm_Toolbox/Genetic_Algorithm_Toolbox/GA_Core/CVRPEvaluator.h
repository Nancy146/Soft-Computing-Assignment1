#pragma once

#include "CVRPDecoder.h"
#include "CVRPProblem.h"
#include "IntegerChromosome.h"
#include "Interfaces.h"

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

namespace ga {
using namespace std;

using DistanceMatrix = vector<vector<double>>;

inline DistanceMatrix euclideanDistances(const vector<pair<double, double>>& coordinates) {
    const size_t nodes = coordinates.size();
    DistanceMatrix distances(nodes, vector<double>(nodes, 0.0));
    for (size_t i = 0; i < nodes; ++i)
        for (size_t j = 0; j < nodes; ++j)
            distances[i][j] = hypot(coordinates[i].first - coordinates[j].first,
                                    coordinates[i].second - coordinates[j].second);
    return distances;
}

struct CVRPEvaluation {
    CVRPDecodedSolution solution;
    double totalDistance = 0.0;

    bool feasible() const { return solution.feasible(); }
};

class CVRPEvaluator {
public:
    CVRPEvaluator(const CVRPProblem& problem, DistanceMatrix distances)
        : problem_(problem), distances_(std::move(distances)) {
        const size_t nodes = problem_.demands.size();
        if (distances_.size() != nodes)
            throw invalid_argument("Distance matrix must have one row per node (depot + customers).");
        for (const auto& row : distances_) {
            if (row.size() != nodes)
                throw invalid_argument("Distance matrix must be square.");
            for (double d : row) {
                if (!(isfinite(d) && d >= 0.0))
                    throw invalid_argument("Distances must be finite and non-negative.");
            }
        }
    }

    double routeDistance(const CVRPDecoder::Route& route) const {
        if (route.empty())
            return 0.0;
        double total = 0.0;
        size_t previous = 0;
        for (int customer : route) {
            const size_t node = checkedCustomer(customer);
            total += distances_[previous][node];
            previous = node;
        }
        return total + distances_[previous][0];
    }

    double totalDistance(const CVRPDecoder::Routes& routes) const {
        double total = 0.0;
        for (const auto& route : routes)
            total += routeDistance(route);
        return total;
    }

    CVRPEvaluation evaluate(const IntegerChromosome& chromosome) const {
        CVRPEvaluation evaluation;
        evaluation.solution = CVRPDecoder::decodeAndCheck(chromosome, problem_);
        evaluation.totalDistance = totalDistance(evaluation.solution.routes);
        return evaluation;
    }

    double totalDistance(const IntegerChromosome& chromosome) const {
        return evaluate(chromosome).totalDistance;
    }

    static double fitnessFromDistance(double distance) { return 1.0 / (1.0 + distance); }

    static double score(const CVRPEvaluation& evaluation) {
        if (evaluation.feasible())
            return fitnessFromDistance(evaluation.totalDistance);
        return -static_cast<double>(evaluation.solution.totalViolation());
    }

    double fitness(const IntegerChromosome& chromosome) const {
        return score(evaluate(chromosome));
    }

    FitnessFunction<IntegerChromosome> fitnessFunction() const {
        const CVRPEvaluator evaluator = *this;
        return [evaluator](const IntegerChromosome& chromosome) { return evaluator.fitness(chromosome); };
    }

    const CVRPProblem& problem() const { return problem_; }
    const DistanceMatrix& distances() const { return distances_; }

private:
    size_t checkedCustomer(int customer) const {
        if (customer < 1 || static_cast<size_t>(customer) > problem_.customerCount())
            throw invalid_argument("Route contains a node that is not a customer (1..N).");
        return static_cast<size_t>(customer);
    }

    CVRPProblem problem_;
    DistanceMatrix distances_;
};

}
