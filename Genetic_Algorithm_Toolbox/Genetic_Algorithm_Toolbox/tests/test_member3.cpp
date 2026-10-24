#include "GeneticAlgorithm.h"
#include "Replacement.h"
#include "MultiPointCrossover.h"
#include "OrderCrossover.h"
#include "SwapMutation.h"
#include "InversionMutation.h"
#include "GaussianMutation.h"
#include "BitFlipMutation.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <numeric>
#include <random>
#include <string>
#include <vector>
using namespace std;
using namespace ga;
static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                      \
    do {                                                                 \
        ++checks;                                                        \
        if (!(cond)) {                                                   \
            ++failures;                                                  \
            cerr << "  FAILED: " #cond "  (line " << __LINE__ << ")\n";  \
        }                                                                \
    } while (0)

static void runTest(const string& name, void (*test)()) {
    const int before = failures;
    test();
    if (failures == before) {
        cout << "[PASS] " << name << '\n';
    }
    else {
        cout << "[FAIL] " << name << '\n';
    }
}

struct IntChrom {
    vector<int> genes;
    vector<int>& getGenes() {
        return genes;
    }
    const vector<int>& getGenes() const {
        return genes;
    }
    bool operator==(const IntChrom& other) const {
        return genes == other.genes;
    }
};

struct FloatChrom {
    vector<double> genes;
    vector<double>& getGenes() {
        return genes;
    }
    const vector<double>& getGenes() const {
        return genes;
    }
};

static IntChrom identityPerm(size_t n) {
    IntChrom c;
    c.genes.resize(n);
    iota(c.genes.begin(), c.genes.end(), 1);
    return c;
}

static IntChrom randomPerm(size_t n, mt19937& rng) {
    IntChrom c = identityPerm(n);
    shuffle(c.genes.begin(), c.genes.end(), rng);
    return c;
}

static bool isPermutation(const IntChrom& c, size_t n) {
    vector<int> sorted = c.genes;
    sort(sorted.begin(), sorted.end());
    return sorted == identityPerm(n).genes;
}

static void testMultiPoint() {
    mt19937 rng(1);
    MultiPointCrossover<IntChrom> crossover(2);
    IntChrom allZeros;
    allZeros.genes.assign(10, 0);
    IntChrom allOnes;
    allOnes.genes.assign(10, 1);
    const IntChrom zerosBefore = allZeros;
    const IntChrom onesBefore = allOnes;
    for (int t = 0; t < 200; ++t) {
        pair<IntChrom, IntChrom> children = crossover.crossover(allZeros, allOnes, rng);
        const IntChrom& c1 = children.first;
        const IntChrom& c2 = children.second;
        CHECK(c1.genes.size() == 10);
        CHECK(c2.genes.size() == 10);
        for (size_t i = 0; i < 10; ++i) {
            CHECK(c1.genes[i] + c2.genes[i] == 1);
        }
        CHECK(c1.genes[0] == 0);
        int changes = 0;
        for (size_t i = 1; i < 10; ++i) {
            if (c1.genes[i] != c1.genes[i - 1]) {
                ++changes;
            }
        }
        CHECK(changes <= 2);
    }
    CHECK(allZeros == zerosBefore);
    CHECK(allOnes == onesBefore);
    MultiPointCrossover<IntChrom> tooManyPoints(50);
    pair<IntChrom, IntChrom> children = tooManyPoints.crossover(allZeros, allOnes, rng);
    CHECK(children.first.genes.size() == 10);
}

static void testOrderCrossover() {
    mt19937 rng(2);
    OrderCrossover<IntChrom> crossover;
    vector<size_t> lengths = {2, 3, 8, 30};
    for (size_t n : lengths) {
        for (int t = 0; t < 300; ++t) {
            const IntChrom parent1 = randomPerm(n, rng);
            const IntChrom parent2 = randomPerm(n, rng);
            const IntChrom parent1Before = parent1;
            const IntChrom parent2Before = parent2;
            pair<IntChrom, IntChrom> children = crossover.crossover(parent1, parent2, rng);
            CHECK(isPermutation(children.first, n));
            CHECK(isPermutation(children.second, n));
            CHECK(parent1 == parent1Before);
            CHECK(parent2 == parent2Before);
        }
    }
}

static void testSwap() {
    mt19937 rng(3);
    SwapMutation<IntChrom> mutation;
    for (int t = 0; t < 300; ++t) {
        IntChrom c = identityPerm(20);
        mutation.mutate(c, 0.1, rng);
        CHECK(isPermutation(c, 20));
    }
    IntChrom changed = identityPerm(200);
    mutation.mutate(changed, 0.1, rng);
    CHECK(!(changed == identityPerm(200)));
    CHECK(isPermutation(changed, 200));
}

static void testInversion() {
    mt19937 rng(4);
    InversionMutation<IntChrom> mutation;
    for (int t = 0; t < 300; ++t) {
        IntChrom c = identityPerm(20);
        mutation.mutate(c, 0.1, rng);
        CHECK(isPermutation(c, 20));
    }
}

static void testGaussian() {
    mt19937 rng(5);
    GaussianMutation<FloatChrom> mutation(0.0, 1.0, 0.5);
    FloatChrom c;
    c.genes.assign(50, 0.5);
    for (int t = 0; t < 200; ++t) {
        mutation.mutate(c, 0.1, rng);
        for (double gene : c.genes) {
            CHECK(gene >= 0.0);
            CHECK(gene <= 1.0);
        }
    }
}

static unsigned int seedWithFirstDrawInPmRange(double& r) {
    for (unsigned int seed = 1;; ++seed) {
        mt19937 probe(seed);
        r = randomUnitInclusive(probe);
        if (r > 0.001 && r < 0.1) {
            return seed;
        }
    }
}

static void testMutationCondition() {
    double r = 0.0;
    const unsigned int seed = seedWithFirstDrawInPmRange(r);
    const double belowR = nextafter(r, 0.0);
    BitFlipMutation bitFlip;

    BinaryChromosome zero(vector<int>{0});
    mt19937 rng1(seed);
    bitFlip.mutate(zero, r, rng1);
    CHECK(zero.getGenes() == vector<int>{1});

    BinaryChromosome one(vector<int>{1});
    mt19937 rng2(seed);
    bitFlip.mutate(one, r, rng2);
    CHECK(one.getGenes() == vector<int>{0});

    BinaryChromosome unchanged(vector<int>{0});
    mt19937 rng3(seed);
    bitFlip.mutate(unchanged, belowR, rng3);
    CHECK(unchanged.getGenes() == vector<int>{0});

    GaussianMutation<FloatChrom> gaussian(0.0, 1.0, 0.1);
    FloatChrom mutated;
    mutated.genes.assign(1, 0.5);
    mt19937 rng4(seed);
    gaussian.mutate(mutated, r, rng4);
    CHECK(mutated.genes[0] != 0.5);
    FloatChrom kept;
    kept.genes.assign(1, 0.5);
    mt19937 rng5(seed);
    gaussian.mutate(kept, belowR, rng5);
    CHECK(kept.genes[0] == 0.5);

    InversionMutation<IntChrom> inversion;
    IntChrom notInverted = identityPerm(20);
    mt19937 rng6(seed);
    mt19937 afterOneDraw(seed);
    randomUnitInclusive(afterOneDraw);
    inversion.mutate(notInverted, belowR, rng6);
    CHECK(notInverted == identityPerm(20));
    CHECK(rng6 == afterOneDraw);
    IntChrom inverted = identityPerm(20);
    mt19937 rng7(seed);
    inversion.mutate(inverted, r, rng7);
    CHECK(!(rng7 == afterOneDraw));
    CHECK(isPermutation(inverted, 20));
}

template <typename C>
static bool rejectsProbability(const MutationStrategy<C>& mutation, C chromosome, double pm) {
    mt19937 rng(6);
    try {
        mutation.mutate(chromosome, pm, rng);
    } catch (const invalid_argument&) {
        return true;
    }
    return false;
}

template <typename C>
static void checkProbabilityRange(const MutationStrategy<C>& mutation, const C& chromosome) {
    CHECK(!rejectsProbability(mutation, chromosome, 0.001));
    CHECK(!rejectsProbability(mutation, chromosome, 0.1));
    CHECK(rejectsProbability(mutation, chromosome, 0.0));
    CHECK(rejectsProbability(mutation, chromosome, 0.0009));
    CHECK(rejectsProbability(mutation, chromosome, nextafter(0.001, 0.0)));
    CHECK(rejectsProbability(mutation, chromosome, nextafter(0.1, 1.0)));
    CHECK(rejectsProbability(mutation, chromosome, 0.11));
    CHECK(rejectsProbability(mutation, chromosome, 1.0));
    CHECK(rejectsProbability(mutation, chromosome, -0.05));
}

static void testMutationProbabilityRange() {
    checkProbabilityRange(BitFlipMutation(), BinaryChromosome(vector<int>{0, 1, 0, 1}));
    checkProbabilityRange(SwapMutation<IntChrom>(), identityPerm(10));
    checkProbabilityRange(InversionMutation<IntChrom>(), identityPerm(10));
    FloatChrom floats;
    floats.genes.assign(10, 0.5);
    checkProbabilityRange(GaussianMutation<FloatChrom>(0.0, 1.0, 0.1), floats);

    CHECK(rejectsProbability(SwapMutation<IntChrom>(), identityPerm(1), 0.5));
    CHECK(rejectsProbability(InversionMutation<IntChrom>(), identityPerm(1), 0.5));
}

static void testCrossoverPointRange() {
    mt19937 rng(8);
    MultiPointCrossover<IntChrom> onePoint(1);
    const size_t L = 10;
    IntChrom zeros;
    zeros.genes.assign(L, 0);
    IntChrom ones;
    ones.genes.assign(L, 1);
    bool sawFirst = false;
    bool sawLast = false;
    for (int t = 0; t < 500; ++t) {
        const IntChrom child = onePoint.crossover(zeros, ones, rng).first;
        const size_t r1 = static_cast<size_t>(find(child.genes.begin(), child.genes.end(), 1) - child.genes.begin());
        CHECK(r1 >= 1);
        CHECK(r1 <= L - 1);
        CHECK(all_of(child.genes.begin() + static_cast<ptrdiff_t>(r1), child.genes.end(), [](int g) { return g == 1; }));
        sawFirst = sawFirst || r1 == 1;
        sawLast = sawLast || r1 == L - 1;
    }
    CHECK(sawFirst);
    CHECK(sawLast);

    IntChrom a;
    a.genes = {0, 0};
    IntChrom b;
    b.genes = {1, 1};
    pair<IntChrom, IntChrom> children = onePoint.crossover(a, b, rng);
    CHECK((children.first.genes == vector<int>{0, 1}));
    CHECK((children.second.genes == vector<int>{1, 0}));

    IntChrom single;
    single.genes = {5};
    IntChrom other;
    other.genes = {7};
    children = onePoint.crossover(single, other, rng);
    CHECK(children.first == single);
    CHECK(children.second == other);
    IntChrom empty;
    children = onePoint.crossover(empty, empty, rng);
    CHECK(children.first.genes.empty());
}

static void testSameSeedSameResult() {
    OrderCrossover<IntChrom> crossover;
    mt19937 rng1(9);
    mt19937 rng2(9);
    const IntChrom a = identityPerm(15);
    IntChrom b = a;
    reverse(b.genes.begin(), b.genes.end());
    IntChrom first = crossover.crossover(a, b, rng1).first;
    IntChrom second = crossover.crossover(a, b, rng2).first;
    CHECK(first == second);
}

static double correctPositions(const IntChrom& c) {
    double score = 0;
    for (size_t i = 0; i < c.genes.size(); ++i) {
        if (c.genes[i] == static_cast<int>(i) + 1) {
            score += 1;
        }
    }
    return score;
}

struct TournamentSelection final : SelectionStrategy<IntChrom> {
    size_t select(const Population<IntChrom>& population,const vector<double>& fitness,mt19937& rng) const override {
        uniform_int_distribution<size_t> pick(0, population.size() - 1);
        size_t a = pick(rng);
        size_t b = pick(rng);
        if (fitness[a] >= fitness[b]) {
            return a;
        }
        return b;
    }
};

struct NoPenalty final : InfeasibilityHandler<IntChrom> {
    double handle(IntChrom&,const FitnessFunction<IntChrom>&,const FeasibilityFunction<IntChrom>&,mt19937&) const override {
        return 0.0;
    }
};
static void runGA(shared_ptr<const MutationStrategy<IntChrom>> mutation) {
    GAConfig config;
    config.populationSize = 60;
    config.maxGenerations = 300;
    config.mutationProbability = 0.05;

    mt19937 rng(11);
    Population<IntChrom> population;
    for (size_t i = 0; i < config.populationSize; ++i) {
        population.push_back(randomPerm(12, rng));
    }
    FeasibilityFunction<IntChrom> alwaysFeasible = [](const IntChrom&) { return true; };
    GeneticAlgorithm<IntChrom> ga(config,population,correctPositions,alwaysFeasible,make_shared<NoPenalty>(),make_shared<TournamentSelection>(),make_shared<OrderCrossover<IntChrom>>(),mutation,make_shared<ElitistReplacement<IntChrom>>());
    GAResult<IntChrom> result = ga.run();
    CHECK(isPermutation(result.bestChromosome, 12));
    CHECK(result.bestHistory.back() > result.bestHistory.front());
}

static void testInsideRealGA() {
    runGA(make_shared<SwapMutation<IntChrom>>());
    runGA(make_shared<InversionMutation<IntChrom>>());
}

int main() {
    runTest("MultiPointCrossover", testMultiPoint);
    runTest("OrderCrossover keeps valid permutations", testOrderCrossover);
    runTest("SwapMutation", testSwap);
    runTest("InversionMutation", testInversion);
    runTest("GaussianMutation respects bounds", testGaussian);
    runTest("Mutation happens when r <= pm, not when r > pm", testMutationCondition);
    runTest("All mutations accept pm only in [0.001, 0.1]", testMutationProbabilityRange);
    runTest("Crossover point r1 is in [1, L-1]", testCrossoverPointRange);
    runTest("Same seed gives same result", testSameSeedSameResult);
    runTest("Operators work inside the real GA", testInsideRealGA);
    cout << '\n' << (checks - failures) << '/' << checks << " checks passed\n";
    if (failures == 0) {
        return 0;
    }
    return 1;
}
