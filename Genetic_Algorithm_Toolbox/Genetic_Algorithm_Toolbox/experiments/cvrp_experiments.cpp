#include "CVRPEvaluator.h"
#include "CVRPExactSolver.h"
#include "CVRPFeasibility.h"
#include "CVRPInstances.h"
#include "CVRPPenaltyHandler.h"
#include "CVRPPermutation.h"
#include "GeneticAlgorithm.h"
#include "InversionMutation.h"
#include "OrderCrossover.h"
#include "Replacement.h"
#include "RouletteWheelSelection.h"
#include "SwapMutation.h"
#include "TournamentSelection.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

using namespace std;
using namespace ga;

const int seeds = 30;

struct Setting {
    string factor;
    string value;
    size_t population;
    size_t generations;
    double pc;
    double pm;
    string selection;
    string mutation;
    string replacement;
};

const vector<Setting> settings = {
    {"baseline", "defaults", 100, 500, 0.6, 0.01, "Tournament", "Swap", "Elitist"},
    {"population", "50", 50, 500, 0.6, 0.01, "Tournament", "Swap", "Elitist"},
    {"population", "200", 200, 500, 0.6, 0.01, "Tournament", "Swap", "Elitist"},
    {"generations", "100", 100, 100, 0.6, 0.01, "Tournament", "Swap", "Elitist"},
    {"generations", "1000", 100, 1000, 0.6, 0.01, "Tournament", "Swap", "Elitist"},
    {"Pc", "0.4", 100, 500, 0.4, 0.01, "Tournament", "Swap", "Elitist"},
    {"Pc", "0.7", 100, 500, 0.7, 0.01, "Tournament", "Swap", "Elitist"},
    {"Pm", "0.001", 100, 500, 0.6, 0.001, "Tournament", "Swap", "Elitist"},
    {"Pm", "0.05", 100, 500, 0.6, 0.05, "Tournament", "Swap", "Elitist"},
    {"Pm", "0.1", 100, 500, 0.6, 0.1, "Tournament", "Swap", "Elitist"},
    {"selection", "Roulette", 100, 500, 0.6, 0.01, "Roulette", "Swap", "Elitist"},
    {"mutation", "Inversion", 100, 500, 0.6, 0.01, "Tournament", "Inversion", "Elitist"},
    {"replacement", "Generational", 100, 500, 0.6, 0.01, "Tournament", "Swap", "Generational"},
};

struct Run {
    bool feasible;
    double distance;
    size_t bestGeneration;
    double milliseconds;
};

Run runGA(const cvrp_instances::Instance& instance, const CVRPEvaluator& evaluator, const Setting& setting, int seed) {
    GAConfig config;
    config.populationSize = setting.population;
    config.maxGenerations = setting.generations;
    config.crossoverProbability = setting.pc;
    config.mutationProbability = setting.pm;
    config.seed = seed;

    shared_ptr<const SelectionStrategy<IntegerChromosome>> selection = make_shared<TournamentSelection<IntegerChromosome>>(3);
    if (setting.selection == "Roulette")
        selection = make_shared<RouletteWheelSelection<IntegerChromosome>>();

    shared_ptr<const MutationStrategy<IntegerChromosome>> mutation = make_shared<SwapMutation<IntegerChromosome>>();
    if (setting.mutation == "Inversion")
        mutation = make_shared<InversionMutation<IntegerChromosome>>();

    shared_ptr<const ReplacementStrategy<IntegerChromosome>> replacement = make_shared<ElitistReplacement<IntegerChromosome>>();
    if (setting.replacement == "Generational")
        replacement = make_shared<GenerationalReplacement<IntegerChromosome>>();

    size_t customers = instance.problem.customerCount();
    mt19937 initRng(seed + 1000);
    GeneticAlgorithm<IntegerChromosome> ga(
        config, randomCustomerPermutations(config.populationSize, customers, initRng),
        evaluator.fitnessFunction(), CVRPFeasibility(instance.problem), make_shared<CVRPFeasibleFirstHandler>(evaluator),
        selection,
        make_shared<PermutationCheckedCrossover>(make_shared<OrderCrossover<IntegerChromosome>>(), customers),
        make_shared<PermutationCheckedMutation>(mutation, customers),
        replacement);

    auto start = chrono::steady_clock::now();
    GAResult<IntegerChromosome> result = ga.run();
    auto stop = chrono::steady_clock::now();

    CVRPEvaluation best = evaluator.evaluate(result.bestChromosome);
    return {best.feasible(), best.totalDistance, result.bestGeneration,
            chrono::duration<double, milli>(stop - start).count()};
}

int main() {
    cout << fixed;
    cout << "# CVRP GA experiments\n\n"
         << "Each setting changes one factor of the baseline (population 100, 500 generations, Pc 0.6, Pm 0.01,\n"
         << "Tournament(3) selection, Swap mutation, Elitist replacement) and is run with seeds 1-" << seeds << ".\n"
         << "Gap = (distance - reference) / reference x 100. Hits = runs that reached the reference.\n"
         << "Best gen = mean generation in which the final best solution was found.\n";

    for (const cvrp_instances::Instance& instance : cvrp_instances::all()) {
        CVRPEvaluator evaluator(instance.problem, instance.distances());

        vector<vector<Run>> runs(settings.size());
        for (size_t s = 0; s < settings.size(); s++)
            for (int seed = 1; seed <= seeds; seed++)
                runs[s].push_back(runGA(instance, evaluator, settings[s], seed));

        bool exact = instance.problem.customerCount() <= 10;
        double reference;
        if (exact) {
            reference = cvrp_exact::solve(instance.problem, instance.distances()).distance;
        } else {
            reference = 1e18;
            for (const vector<Run>& settingRuns : runs)
                for (const Run& run : settingRuns)
                    if (run.feasible)
                        reference = min(reference, run.distance);
        }

        cout << "\n## " << instance.name << " (" << instance.problem.customerCount() << " customers, "
             << instance.problem.vehicleCount << " vehicles of capacity " << instance.problem.vehicleCapacity
             << "; reference " << setprecision(4) << reference << (exact ? ", exact optimum" : ", best distance found") << ")\n\n"
             << "| Factor | Setting | Feasible | Hits | Mean gap % | Worst gap % | Best gen | Time ms |\n"
             << "|---|---|---|---|---|---|---|---|\n";

        for (size_t s = 0; s < settings.size(); s++) {
            int feasible = 0;
            int hits = 0;
            double gapSum = 0;
            double worstGap = 0;
            double generationSum = 0;
            double timeSum = 0;
            for (const Run& run : runs[s]) {
                timeSum += run.milliseconds;
                if (!run.feasible)
                    continue;
                double gap = 100 * (run.distance - reference) / reference;
                feasible++;
                if (gap < 1e-7)
                    hits++;
                gapSum += gap;
                worstGap = max(worstGap, gap);
                generationSum += run.bestGeneration;
            }

            cout << "| " << settings[s].factor << " | " << settings[s].value << " | " << feasible << "/" << seeds << " | "
                 << hits << "/" << seeds << " | ";
            if (feasible > 0)
                cout << setprecision(2) << gapSum / feasible << " | " << worstGap << " | " << setprecision(0)
                     << generationSum / feasible << " | ";
            else
                cout << "- | - | - | ";
            cout << setprecision(1) << timeSum / seeds << " |\n";
        }
    }
    return 0;
}
