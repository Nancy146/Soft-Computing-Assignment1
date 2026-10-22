#include "GAConfig.h"
#include "GeneticAlgorithm.h"
#include "Replacement.h"

#include <cmath>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace ga;

// ───────────────────────────── tiny test runner ─────────────────────────────
static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++checks;                                                                \
        if (!(cond)) {                                                           \
            ++failures;                                                          \
            cerr << "  FAILED: " #cond "  (" << __FILE__ << ":" << __LINE__ << ")\n"; \
        }                                                                        \
    } while (0)

template <typename Exception, typename Fn>
static bool throwsType(Fn fn) {
    try { fn(); } catch (const Exception&) { return true; } catch (...) { return false; }
    return false;
}

static void runTest(const string& name, const function<void()>& test) {
    const int before = failures;
    try {
        test();
    } catch (const exception& e) {
        ++failures;
        cerr << "  FAILED: unexpected exception: " << e.what() << '\n';
    }
    cout << (failures == before ? "[PASS] " : "[FAIL] ") << name << '\n';
}

// ───────────────────────────── test stubs ─────────────────────────────
struct Bits {
    vector<int> genes;
    bool operator==(const Bits& other) const { return genes == other.genes; }
};

struct RandomSelection final : SelectionStrategy<Bits> {
    size_t select(const Population<Bits>& population, const vector<double>&, mt19937& rng) const override {
        return uniform_int_distribution<size_t>(0, population.size() - 1)(rng);
    }
};

struct TournamentSelection2 final : SelectionStrategy<Bits> {
    size_t select(const Population<Bits>& population, const vector<double>& fitness, mt19937& rng) const override {
        uniform_int_distribution<size_t> pick(0, population.size() - 1);
        const size_t a = pick(rng), b = pick(rng);
        return fitness[a] >= fitness[b] ? a : b;
    }
};

struct OnePointCrossover final : CrossoverStrategy<Bits> {
    mutable size_t calls = 0;
    pair<Bits, Bits> crossover(const Bits& p1, const Bits& p2, mt19937& rng) const override {
        ++calls;
        Bits c1 = p1, c2 = p2;
        const size_t cut = uniform_int_distribution<size_t>(1, p1.genes.size() - 1)(rng);
        for (size_t i = cut; i < c1.genes.size(); ++i) swap(c1.genes[i], c2.genes[i]);
        return {c1, c2};
    }
};

struct BitFlipMutation final : MutationStrategy<Bits> {
    void mutate(Bits& c, double pm, mt19937& rng) const override {
        uniform_real_distribution<double> unit(0.0, 1.0);
        for (int& g : c.genes)
            if (unit(rng) < pm) g = 1 - g;
    }
};

struct NoMutation final : MutationStrategy<Bits> {
    void mutate(Bits&, double, mt19937&) const override {}
};

struct ConstantHandler final : InfeasibilityHandler<Bits> {
    double value;
    mutable size_t calls = 0;
    explicit ConstantHandler(double v) : value(v) {}
    double handle(Bits&, const FitnessFunction<Bits>&, const FeasibilityFunction<Bits>&, mt19937&) const override {
        ++calls;
        return value;
    }
};

// ───────────────────────────── helpers ─────────────────────────────
static double oneMax(const Bits& c) {
    double s = 0;
    for (int g : c.genes) s += g;
    return s;
}

static Population<Bits> randomPopulation(size_t n, size_t length, unsigned int seed) {
    mt19937 rng(seed);
    Population<Bits> population(n);
    for (Bits& c : population) {
        c.genes.resize(length);
        for (int& g : c.genes) g = static_cast<int>(rng() % 2);
    }
    return population;
}

static GAConfig smallConfig() {
    GAConfig config;
    config.populationSize = 30;
    config.maxGenerations = 60;
    config.mutationProbability = 0.02;
    return config;
}

struct Setup {
    GAConfig config = smallConfig();
    Population<Bits> population = randomPopulation(30, 32, 7);
    FitnessFunction<Bits> fitness = oneMax;
    FeasibilityFunction<Bits> feasibility = [](const Bits&) { return true; };
    shared_ptr<const InfeasibilityHandler<Bits>> handler = make_shared<ConstantHandler>(0.0);
    shared_ptr<const SelectionStrategy<Bits>> selection = make_shared<RandomSelection>();
    shared_ptr<const CrossoverStrategy<Bits>> crossover = make_shared<OnePointCrossover>();
    shared_ptr<const MutationStrategy<Bits>> mutation = make_shared<BitFlipMutation>();
    shared_ptr<const ReplacementStrategy<Bits>> replacement = make_shared<ElitistReplacement<Bits>>();

    GeneticAlgorithm<Bits> build() const {
        return GeneticAlgorithm<Bits>(config, population, fitness, feasibility, handler,
                                      selection, crossover, mutation, replacement);
    }
};

// ───────────────────────────── tests ─────────────────────────────
static void testConfigValidation() {
    CHECK(!throwsType<invalid_argument>([] { GAConfig{}.validate(); }));

    auto invalid = [](function<void(GAConfig&)> change) {
        GAConfig c;
        change(c);
        return throwsType<invalid_argument>([&] { c.validate(); });
    };
    CHECK(invalid([](GAConfig& c) { c.populationSize = 0; }));
    CHECK(invalid([](GAConfig& c) { c.maxGenerations = 0; }));
    CHECK(invalid([](GAConfig& c) { c.crossoverProbability = 1.1; }));
    CHECK(invalid([](GAConfig& c) { c.crossoverProbability = -0.1; }));
    CHECK(invalid([](GAConfig& c) { c.mutationProbability = -0.1; }));
    CHECK(invalid([](GAConfig& c) { c.mutationProbability = 1.5; }));
    CHECK(invalid([](GAConfig& c) { c.mutationProbability = numeric_limits<double>::quiet_NaN(); }));

    // Boundary values are valid.
    CHECK(!invalid([](GAConfig& c) { c.crossoverProbability = 0.0; c.mutationProbability = 1.0; }));
}

static void testConstructorRejectsBadInput() {
    {
        Setup s;
        s.config.populationSize = 10;   // population has 30
        CHECK(throwsType<invalid_argument>([&] { s.build(); }));
    }
    {
        Setup s;
        s.selection = nullptr;
        CHECK(throwsType<invalid_argument>([&] { s.build(); }));
    }
    {
        Setup s;
        s.handler = nullptr;
        CHECK(throwsType<invalid_argument>([&] { s.build(); }));
    }
    {
        Setup s;
        s.fitness = nullptr;
        CHECK(throwsType<invalid_argument>([&] { s.build(); }));
    }
}

static void testSameSeedSameResult() {
    Setup s;
    const auto a = s.build().run();
    const auto b = s.build().run();
    CHECK(a.bestFitness == b.bestFitness);
    CHECK(a.bestChromosome == b.bestChromosome);
    CHECK(a.bestGeneration == b.bestGeneration);
    CHECK(a.bestHistory == b.bestHistory);
    CHECK(a.averageHistory == b.averageHistory);

    // Calling run() twice on the same object also gives the same result.
    const auto ga = s.build();
    CHECK(ga.run().averageHistory == ga.run().averageHistory);
}

static void testHistoryLength() {
    Setup s;
    const auto result = s.build().run();
    CHECK(result.bestHistory.size() == s.config.maxGenerations + 1);
    CHECK(result.averageHistory.size() == s.config.maxGenerations + 1);
}

static void testElitismNeverLosesBest() {
    for (unsigned int seed = 1; seed <= 20; ++seed) {
        Setup s;
        s.config.seed = seed;
        s.config.mutationProbability = 0.2;   // heavy mutation: offspring are often worse
        s.replacement = make_shared<ElitistReplacement<Bits>>();
        const auto result = s.build().run();
        for (size_t g = 1; g < result.bestHistory.size(); ++g)
            CHECK(result.bestHistory[g] >= result.bestHistory[g - 1]);
    }
}

static void testGaSolvesOneMax() {
    for (auto replacement : {shared_ptr<const ReplacementStrategy<Bits>>(make_shared<ElitistReplacement<Bits>>()),
                             shared_ptr<const ReplacementStrategy<Bits>>(make_shared<GenerationalReplacement<Bits>>())}) {
        Setup s;
        s.config.maxGenerations = 150;
        s.selection = make_shared<TournamentSelection2>();
        s.replacement = replacement;
        const auto result = s.build().run();
        CHECK(result.bestFitness >= 28);                               // out of 32
        CHECK(result.bestHistory.back() > result.bestHistory.front());
        CHECK(result.bestIsFeasible);
        CHECK(oneMax(result.bestChromosome) == result.bestFitness);    // fitness matches the chromosome
    }
}

static void testFitnessEvaluatedOncePerNewIndividual() {
    Setup s;
    size_t calls = 0;
    s.fitness = [&calls](const Bits& c) { ++calls; return oneMax(c); };
    for (auto replacement : {shared_ptr<const ReplacementStrategy<Bits>>(make_shared<ElitistReplacement<Bits>>()),
                             shared_ptr<const ReplacementStrategy<Bits>>(make_shared<GenerationalReplacement<Bits>>())}) {
        calls = 0;
        s.replacement = replacement;
        s.build().run();
        // N initial evaluations + N offspring per generation; survivors are never re-evaluated.
        CHECK(calls == s.config.populationSize * (s.config.maxGenerations + 1));
    }
}

static void testCrossoverProbabilityDecidedByCore() {
    for (double pc : {0.0, 1.0}) {
        Setup s;
        s.config.crossoverProbability = pc;
        auto crossover = make_shared<OnePointCrossover>();
        s.crossover = crossover;
        s.build().run();
        const size_t pairsPerGeneration = (s.config.populationSize + 1) / 2;
        CHECK(crossover->calls == (pc == 0.0 ? 0 : pairsPerGeneration * s.config.maxGenerations));
    }
}

static void testInfeasibleGoesToHandler() {
    Setup s;
    s.config.maxGenerations = 5;
    size_t infeasible = 0;
    // Infeasible if the first gene is 1.
    s.feasibility = [&infeasible](const Bits& c) {
        const bool ok = c.genes[0] == 0;
        if (!ok) ++infeasible;
        return ok;
    };
    size_t fitnessCalls = 0;
    s.fitness = [&fitnessCalls](const Bits& c) { ++fitnessCalls; return oneMax(c); };
    auto handler = make_shared<ConstantHandler>(0.5);
    s.handler = handler;

    const auto result = s.build().run();
    const size_t evaluated = s.config.populationSize * (s.config.maxGenerations + 1);

    // The final feasibility check on the best chromosome is also counted in `infeasible`.
    const size_t finalCheckInfeasible = result.bestIsFeasible ? 0 : 1;
    CHECK(handler->calls == infeasible - finalCheckInfeasible);
    CHECK(fitnessCalls + handler->calls == evaluated);
    CHECK(handler->calls > 0);
}

static void testGenerationalReplacement() {
    GenerationalReplacement<Bits> replacement;
    const Population<Bits> current = randomPopulation(5, 8, 1);
    const Population<Bits> offspring = randomPopulation(5, 8, 2);
    const vector<double> currentFitness{9, 9, 9, 9, 9};
    const vector<double> offspringFitness{1, 2, 3, 4, 5};

    const auto next = replacement.replace(current, currentFitness, offspring, offspringFitness);
    CHECK(next.population.size() == current.size());
    CHECK(next.population == offspring);
    CHECK(next.fitness == offspringFitness);

    CHECK(throwsType<invalid_argument>([&] {
        replacement.replace(current, currentFitness, randomPopulation(4, 8, 3), {1, 2, 3, 4});
    }));
}

static void testElitistReplacement() {
    ElitistReplacement<Bits> replacement;
    const Population<Bits> current = randomPopulation(5, 8, 1);
    const Population<Bits> offspring = randomPopulation(5, 8, 2);
    const vector<double> currentFitness{3, 10, 2, 1, 4};      // best = index 1
    const vector<double> offspringFitness{5, 1, 7, 6, 2};     // all worse than 10

    const auto next = replacement.replace(current, currentFitness, offspring, offspringFitness);
    CHECK(next.population.size() == current.size());
    CHECK(next.fitness.size() == current.size());

    // Best current individual is kept.
    CHECK(next.population[0] == current[1]);
    CHECK(next.fitness[0] == 10);

    // Remaining slots are the best 4 offspring: 7, 6, 5, 2.
    CHECK((vector<double>(next.fitness.begin() + 1, next.fitness.end()) == vector<double>{7, 6, 5, 2}));
    CHECK(next.population[1] == offspring[2]);
    CHECK(next.population[2] == offspring[3]);
    CHECK(next.population[3] == offspring[0]);
    CHECK(next.population[4] == offspring[4]);
}

static void testCoreRejectsBrokenComponents() {
    struct WrongSizeReplacement final : ReplacementStrategy<Bits> {
        ReplacementResult<Bits> replace(const Population<Bits>& current, const vector<double>& fitness,
                                        const Population<Bits>&, const vector<double>&) const override {
            return {Population<Bits>(current.begin(), current.end() - 1),
                    vector<double>(fitness.begin(), fitness.end() - 1)};
        }
    };
    struct OutOfRangeSelection final : SelectionStrategy<Bits> {
        size_t select(const Population<Bits>& population, const vector<double>&, mt19937&) const override {
            return population.size();
        }
    };
    {
        Setup s;
        s.replacement = make_shared<WrongSizeReplacement>();
        CHECK(throwsType<logic_error>([&] { s.build().run(); }));
    }
    {
        Setup s;
        s.selection = make_shared<OutOfRangeSelection>();
        CHECK(throwsType<out_of_range>([&] { s.build().run(); }));
    }
}

int main() {
    runTest("GAConfig validation", testConfigValidation);
    runTest("Constructor rejects bad input", testConstructorRejectsBadInput);
    runTest("Same seed gives same result", testSameSeedSameResult);
    runTest("History has one entry per generation", testHistoryLength);
    runTest("Elitist: best fitness never decreases", testElitismNeverLosesBest);
    runTest("GA improves OneMax with both replacements", testGaSolvesOneMax);
    runTest("Fitness evaluated once per new individual", testFitnessEvaluatedOncePerNewIndividual);
    runTest("Crossover probability decided by the core", testCrossoverProbabilityDecidedByCore);
    runTest("Infeasible chromosomes go to the handler", testInfeasibleGoesToHandler);
    runTest("GenerationalReplacement", testGenerationalReplacement);
    runTest("ElitistReplacement", testElitistReplacement);
    runTest("Core rejects broken components", testCoreRejectsBrokenComponents);

    cout << '\n' << (checks - failures) << '/' << checks << " checks passed\n";
    return failures == 0 ? 0 : 1;
}
