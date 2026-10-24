#include <algorithm>
#include <cassert>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

#include "BinaryChromosome.h"
#include "IntegerChromosome.h"
#include "FloatChromosome.h"

#include "TournamentSelection.h"
#include "RouletteWheelSelection.h"

#include "BitFlipMutation.h"

using namespace std;
using namespace ga;

int main()
{
    mt19937 rng(42);

    BinaryChromosome binary(20);

    binary.initialize(rng);

    assert(binary.size() == 20);
    assert(binary.isValid());

    assert(all_of(binary.getGenes().begin(), binary.getGenes().end(),
                  [](int gene) { return gene == 0 || gene == 1; }));

    cout << "Binary chromosome test passed.\n";

    IntegerChromosome integerChromosome(
        20,
        0,
        100
    );

    integerChromosome.initialize(rng);

    assert(integerChromosome.size() == 20);
    assert(integerChromosome.isValid());

    assert(all_of(integerChromosome.getGenes().begin(), integerChromosome.getGenes().end(),
                  [](int gene) { return gene >= 0 && gene <= 100; }));

    cout << "Integer chromosome test passed.\n";

    FloatChromosome floatChromosome(
        20,
        -1.0,
        1.0
    );

    floatChromosome.initialize(rng);

    assert(floatChromosome.size() == 20);
    assert(floatChromosome.isValid());

    assert(all_of(floatChromosome.getGenes().begin(), floatChromosome.getGenes().end(),
                  [](double gene) { return gene >= -1.0 && gene <= 1.0; }));

    cout << "Float chromosome test passed.\n";

    Population<BinaryChromosome> population(5, BinaryChromosome(4));

    vector<double> fitness = {
        10.0,
        50.0,
        20.0,
        5.0,
        30.0
    };

    TournamentSelection<BinaryChromosome> tournament(3);

    for (int i = 0; i < 100; ++i) {
        assert(tournament.select(population, fitness, rng) < population.size());
    }

    cout << "Tournament selection test passed.\n";

    RouletteWheelSelection<BinaryChromosome> roulette;

    for (int i = 0; i < 100; ++i) {
        assert(roulette.select(population, fitness, rng) < population.size());
    }

    cout << "Roulette wheel selection test passed.\n";

    BinaryChromosome mutationChromosome(
        vector<int>(1000, 0)
    );

    BitFlipMutation mutation;

    mutation.mutate(
        mutationChromosome,
        0.1,
        rng
    );

    [[maybe_unused]] const auto flipped =
        count(mutationChromosome.getGenes().begin(), mutationChromosome.getGenes().end(), 1);
    assert(flipped > 0 && flipped < 1000);
    assert(mutationChromosome.isValid());

    cout << "Bit flip mutation test passed.\n";

    BinaryChromosome unchanged(
        vector<int>{1, 0, 1, 0}
    );

    [[maybe_unused]] bool rejected = true;
    for (double probability : {0.0, 1.0}) {
        try {
            mutation.mutate(unchanged, probability, rng);
            rejected = false;
        } catch (const invalid_argument&) {
        }
    }

    assert(rejected);
    assert((unchanged.getGenes() == vector<int>{1, 0, 1, 0}));

    cout << "Mutation probability test passed.\n";

    cout << "\nAll Member 2 tests passed successfully!\n";

    return 0;
}
