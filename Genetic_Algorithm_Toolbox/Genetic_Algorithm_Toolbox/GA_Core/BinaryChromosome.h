#pragma once

#include <cstddef>
#include <random>
#include <vector>

namespace ga {

class BinaryChromosome {
private:
    std::vector<int> genes;

public:
    explicit BinaryChromosome(std::size_t length);

    BinaryChromosome(const std::vector<int>& genes);

    void initialize(std::mt19937& rng);

    std::vector<int>& getGenes();
    const std::vector<int>& getGenes() const;

    std::size_t size() const;

    bool isValid() const;
};

}
