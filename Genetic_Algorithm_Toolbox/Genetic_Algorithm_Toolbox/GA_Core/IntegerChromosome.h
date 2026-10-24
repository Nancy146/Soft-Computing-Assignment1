#pragma once

#include <cstddef>
#include <random>
#include <vector>

namespace ga {

using namespace std;

class IntegerChromosome {
private:
    vector<int> genes;
    int minValue;
    int maxValue;

public:
    IntegerChromosome(
        size_t length,
        int minValue,
        int maxValue
    );

    IntegerChromosome(
        const vector<int>& genes,
        int minValue,
        int maxValue
    );

    void initialize(mt19937& rng);

    vector<int>& getGenes();
    const vector<int>& getGenes() const;

    size_t size() const;

    bool isValid() const;

    int getMinValue() const;
    int getMaxValue() const;
};

}
