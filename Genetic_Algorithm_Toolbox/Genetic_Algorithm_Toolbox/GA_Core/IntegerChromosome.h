#pragma once

#include <cstddef>
#include <random>
#include <vector>

namespace ga {

class IntegerChromosome {
private:
    std::vector<int> genes;
    int minValue;
    int maxValue;

public:
    IntegerChromosome(
        std::size_t length,
        int minValue,
        int maxValue
    );

    IntegerChromosome(
        const std::vector<int>& genes,
        int minValue,
        int maxValue
    );

    void initialize(std::mt19937& rng);

    std::vector<int>& getGenes();
    const std::vector<int>& getGenes() const;

    std::size_t size() const;

    bool isValid() const;

    int getMinValue() const;
    int getMaxValue() const;
};

}