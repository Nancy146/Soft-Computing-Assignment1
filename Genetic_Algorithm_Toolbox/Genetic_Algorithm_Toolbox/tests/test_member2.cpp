#include <cassert>
#include <iostream>
#include <random>
#include <vector>

#include "../BinaryChromosome.h"
#include "../IntegerChromosome.h"
#include "../FloatChromosome.h"

#include "../TournamentSelection.h"
#include "../RouletteWheelSelection.h"

#include "../BitFlipMutation.h"

using namespace std;
using namespace ga;

int main()
{
    mt19937 rng(42);

    // ==========================================
    // Binary Chromosome
    // ==========================================

    BinaryChromosome binary(20);

    binary.initialize(rng);

    assert(binary.size() == 20);
    assert(binary.isValid());

    for (int gene : binary.getGenes()) {
        assert(gene == 0 || gene == 1);
    }

    cout << "Binary chromosome test passed.\n";


    // ==========================================
    // Integer Chromosome
    // ==========================================

    IntegerChromosome integerChromosome(
        20,
        0,
        100
    );

    integerChromosome.initialize(rng);

    assert(integerChromosome.size() == 20);
    assert(integerChromosome.isValid());

    for (int gene : integerChromosome.getGenes()) {
        assert(gene >= 0);
        assert(gene <= 100);
    }

    cout << "Integer chromosome test passed.\n";


    // ==========================================
    // Float Chromosome
    // ==========================================

    FloatChromosome floatChromosome(
        20,
        -1.0,
        1.0
    );

    floatChromosome.initialize(rng);

    assert(floatChromosome.size() == 20);
    assert(floatChromosome.isValid());

    for (double gene : floatChromosome.getGenes()) {
        assert(gene >= -1.0);
        assert(gene <= 1.0);
    }

    cout << "Float chromosome test passed.\n";


    // ==========================================
    // Tournament Selection
    // ==========================================

    Population<BinaryChromosome> population(5);

    vector<double> fitness = {
        10.0,
        50.0,
        20.0,
        5.0,
        30.0
    };

    TournamentSelection<BinaryChromosome> tournament(3);

    for (int i = 0; i < 100; ++i) {

        size_t selected =
            tournament.select(
                population,
                fitness,
                rng
            );

        assert(selected < population.size());
    }

    cout << "Tournament selection test passed.\n";


    // ==========================================
    // Roulette Wheel Selection
    // ==========================================

    RouletteWheelSelection<BinaryChromosome> roulette;

    for (int i = 0; i < 100; ++i) {

        size_t selected =
            roulette.select(
                population,
                fitness,
                rng
            );

        assert(selected < population.size());
    }

    cout << "Roulette wheel selection test passed.\n";


    // ==========================================
    // Bit Flip Mutation
    // ==========================================

    BinaryChromosome mutationChromosome(
        vector<int>{0, 0, 0, 0, 0}
    );

    BitFlipMutation mutation;

    // Probability = 1
    // Every bit MUST flip.
    mutation.mutate(
        mutationChromosome,
        1.0,
        rng
    );

    for (int gene : mutationChromosome.getGenes()) {
        assert(gene == 1);
    }

    cout << "Bit flip mutation test passed.\n";


    // ==========================================
    // Probability = 0
    // Nothing should change.
    // ==========================================

    BinaryChromosome unchanged(
        vector<int>{1, 0, 1, 0}
    );

    mutation.mutate(
        unchanged,
        0.0,
        rng
    );

    assert(
        unchanged.getGenes()
        == vector<int>{1, 0, 1, 0}
    );

    cout << "Mutation probability test passed.\n";


    cout << "\nAll Member 2 tests passed successfully!\n";

    return 0;
}