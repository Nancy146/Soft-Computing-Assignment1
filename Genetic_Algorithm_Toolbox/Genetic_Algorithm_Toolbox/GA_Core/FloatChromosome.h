#pragma once

#include <cstddef>
#include <random>
#include <vector>

namespace ga {

using namespace std;

class FloatChromosome {
private:
    vector<double> genes;
    double minValue;
    double maxValue;

public:
    FloatChromosome(
        size_t length,
        double minValue,
        double maxValue
    );

    FloatChromosome(
        const vector<double>& genes,
        double minValue,
        double maxValue
    );

    void initialize(mt19937& rng);

    vector<double>& getGenes();
    const vector<double>& getGenes() const;

    size_t size() const;

    bool isValid() const;

    double getMinValue() const;
    double getMaxValue() const;
};

}
