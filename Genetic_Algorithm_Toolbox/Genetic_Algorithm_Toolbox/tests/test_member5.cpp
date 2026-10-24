#include "CVRPDecoder.h"
#include "CVRPEvaluator.h"
#include "CVRPExactSolver.h"
#include "CVRPFeasibility.h"
#include "CVRPInstances.h"
#include "CVRPPenaltyHandler.h"
#include "CVRPPermutation.h"
#include "GeneticAlgorithm.h"
#include "MultiPointCrossover.h"
#include "OrderCrossover.h"
#include "Replacement.h"
#include "SwapMutation.h"
#include "TournamentSelection.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <vector>

using namespace std;
using namespace ga;

[[maybe_unused]] static bool near(double a, double b) { return fabs(a - b) < 1e-9; }

template <typename Fn>
static bool throwsInvalidArgument(Fn fn) {
    try { fn(); } catch (const invalid_argument&) { return true; }
    return false;
}

[[maybe_unused]] static double distanceFromCoordinates(const CVRPDecoder::Routes& routes, const cvrp_instances::Instance& instance) {
    double total = 0.0;
    for (const auto& route : routes) {
        vector<int> path{0};
        path.insert(path.end(), route.begin(), route.end());
        path.push_back(0);
        for (size_t i = 0; i + 1 < path.size(); ++i) {
            const auto& a = instance.coordinates[static_cast<size_t>(path[i])];
            const auto& b = instance.coordinates[static_cast<size_t>(path[i + 1])];
            total += hypot(a.first - b.first, a.second - b.second);
        }
    }
    return total;
}

static void testGAOnInstance(const cvrp_instances::Instance& instance) {
    const CVRPEvaluator evaluator(instance.problem, instance.distances());
    const size_t customers = instance.problem.customerCount();
    GAConfig config;
    mt19937 rng(config.seed);

    GeneticAlgorithm<IntegerChromosome> geneticAlgorithm(
        config, randomCustomerPermutations(config.populationSize, customers, rng),
        evaluator.fitnessFunction(), CVRPFeasibility(instance.problem),
        make_shared<CVRPFeasibleFirstHandler>(evaluator),
        make_shared<TournamentSelection<IntegerChromosome>>(3),
        make_shared<PermutationCheckedCrossover>(make_shared<OrderCrossover<IntegerChromosome>>(), customers),
        make_shared<PermutationCheckedMutation>(make_shared<SwapMutation<IntegerChromosome>>(), customers),
        make_shared<ElitistReplacement<IntegerChromosome>>());
    const GAResult<IntegerChromosome> result = geneticAlgorithm.run();

    const CVRPEvaluation best = evaluator.evaluate(result.bestChromosome);
    assert(result.bestIsFeasible && best.feasible());
    assert(isCustomerPermutation(result.bestChromosome, customers));
    assert(best.solution.routes.size() <= static_cast<size_t>(instance.problem.vehicleCount));
    for ([[maybe_unused]] int load : best.solution.routeLoads)
        assert(load <= instance.problem.vehicleCapacity);
    assert(near(distanceFromCoordinates(best.solution.routes, instance), best.totalDistance));
    assert(near(result.bestFitness, 1.0 / (1.0 + best.totalDistance)));

    const cvrp_exact::Solution optimum = cvrp_exact::solve(instance.problem, instance.distances());
    assert(best.totalDistance >= optimum.distance - 1e-9);
    cout << instance.name << ": GA " << best.totalDistance << ", exact optimum " << optimum.distance << '\n';
}

int main() {
    const DistanceMatrix triangle = euclideanDistances({{0, 0}, {3, 0}, {3, 4}});
    assert(near(triangle[0][1], 3) && near(triangle[1][2], 4) && near(triangle[0][2], 5));
    const CVRPEvaluator small(CVRPProblem({0, 1, 1}, 5, 2), triangle);
    assert(near(small.routeDistance({1, 2}), 3 + 4 + 5));
    assert(near(small.routeDistance({1}), 3 + 3));
    assert(small.routeDistance({}) == 0.0);
    assert(near(small.totalDistance(CVRPDecoder::Routes{{1}, {2}}), 6 + 10));

    const DistanceMatrix oneWay = {{0, 1, 3}, {10, 0, 2}, {40, 20, 0}};
    const CVRPEvaluator directed(CVRPProblem({0, 1, 1}, 5, 2), oneWay);
    assert(near(directed.routeDistance({1, 2}), 1 + 2 + 40));
    assert(near(directed.routeDistance({2, 1}), 3 + 20 + 10));

    const CVRPProblem line({0, 2, 3, 4, 2}, 5, 3);
    const CVRPEvaluator evaluator(line, euclideanDistances({{0, 0}, {1, 0}, {2, 0}, {0, 3}, {0, -4}}));
    const IntegerChromosome good({1, 2, 3, 4}, 1, 4);
    const IntegerChromosome longer({2, 4, 1, 3}, 1, 4);
    assert((CVRPDecoder::decode(good, line) == CVRPDecoder::Routes{{1, 2}, {3}, {4}}));
    assert(near(evaluator.totalDistance(good), 18.0));
    assert(near(evaluator.fitness(good), 1.0 / 19.0));
    assert(near(evaluator.totalDistance(longer), 14.0 + sqrt(20.0)));
    assert(evaluator.fitness(good) > evaluator.fitness(longer));

    const CVRPDecodedSolution duplicate = CVRPDecoder::decodeAndCheck(IntegerChromosome({1, 2, 2, 4}, 1, 4), line);
    assert(duplicate.duplicateCustomers == 1 && duplicate.missingCustomers == 1 && duplicate.totalViolation() == 2);
    const CVRPDecodedSolution unknown = CVRPDecoder::decodeAndCheck(IntegerChromosome({1, 9, 2}, 0, 10), line);
    assert(unknown.invalidGenes == 1 && unknown.missingCustomers == 2 && unknown.totalViolation() == 3);
    const CVRPProblem twoVehicles({0, 2, 3, 4, 2}, 5, 2);
    assert(CVRPDecoder::decodeAndCheck(good, twoVehicles).extraVehicles == 1);
    assert(!CVRPFeasibility(twoVehicles)(good));

    assert(evaluator.fitness(IntegerChromosome({1, 2, 2, 4}, 1, 4)) == -2.0);
    assert(CVRPEvaluator(twoVehicles, evaluator.distances()).fitness(good) == -1.0);
    IntegerChromosome infeasible({1, 2, 2, 4}, 1, 4);
    mt19937 rng(1);
    assert(CVRPFeasibleFirstHandler(evaluator).handle(infeasible, evaluator.fitnessFunction(),
                                                      CVRPFeasibility(line), rng) == -2.0);

    assert(throwsInvalidArgument([&] { CVRPEvaluator(line, DistanceMatrix(4, vector<double>(5, 1.0))); }));
    assert(throwsInvalidArgument([&] { DistanceMatrix d = evaluator.distances(); d[1][2] = -1; CVRPEvaluator(line, d); }));
    assert(throwsInvalidArgument([&] { evaluator.routeDistance({1, 0, 2}); }));

    assert(isCustomerPermutation(IntegerChromosome({3, 1, 2}, 0, 9), 3));
    assert(!isCustomerPermutation(IntegerChromosome({1, 2, 2}, 0, 9), 3));
    for ([[maybe_unused]] const IntegerChromosome& c : randomCustomerPermutations(50, 7, rng))
        assert(isCustomerPermutation(c, 7));
    const PermutationCheckedCrossover multiPoint(make_shared<MultiPointCrossover<IntegerChromosome>>(2), 4);
    bool caught = false;
    for (int t = 0; t < 100 && !caught; ++t)
        try { multiPoint.crossover(good, longer, rng); } catch (const logic_error&) { caught = true; }
    assert(caught);

    assert(near(cvrp_exact::solve(CVRPProblem({0, 1, 1}, 5, 2), triangle).distance, 12.0));
    assert(near(cvrp_exact::solve(CVRPProblem({0, 1, 1}, 1, 2), triangle).distance, 16.0));
    assert(!cvrp_exact::solve(CVRPProblem({0, 1, 1}, 1, 1), triangle).feasible);
    assert(near(cvrp_exact::solve(line, evaluator.distances()).distance, 18.0));

    testGAOnInstance(cvrp_instances::a8());
    testGAOnInstance(cvrp_instances::c9Tight());

    cout << "Member 5 CVRP tests passed.\n";
    return 0;
}
