#pragma once

#include <cstddef>
#include <random>
#include <vector>

namespace ga {

class FloatChromosome {
private:
    std::vector<double> genes;
    double minValue;
    double maxValue;

public:
    FloatChromosome(
        std::size_t length,
        double minValue,
        double maxValue
    );

    FloatChromosome(
        const std::vector<double>& genes,
        double minValue,
        double maxValue
    );

    void initialize(std::mt19937& rng);

    std::vector<double>& getGenes();
    const std::vector<double>& getGenes() const;

    std::size_t size() const;

    bool isValid() const;

    double getMinValue() const;
    double getMaxValue() const;
};

}