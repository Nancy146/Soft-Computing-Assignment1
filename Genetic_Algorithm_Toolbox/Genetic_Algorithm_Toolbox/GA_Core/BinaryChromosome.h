#pragma once

#include <cstddef>
#include <random>
#include <vector>

namespace ga {

using namespace std;

class BinaryChromosome {
private:
    vector<int> genes;

public:
    explicit BinaryChromosome(size_t length);

    BinaryChromosome(const vector<int>& genes);

    void initialize(mt19937& rng);

    vector<int>& getGenes();
    const vector<int>& getGenes() const;

    size_t size() const;

    bool isValid() const;
};

}
