#pragma once
#include <cstddef>
#include <stdexcept>

namespace ga {

using namespace std;

struct GAConfig {
    size_t populationSize = 100;
    size_t maxGenerations = 500;
    double crossoverProbability = 0.7;
    double mutationProbability = 0.1;
    unsigned int seed = 42;

    void validate() const {
        if (populationSize == 0)
            throw invalid_argument("populationSize must be greater than zero.");
        if (maxGenerations == 0)
            throw invalid_argument("maxGenerations must be greater than zero.");
        if (!(crossoverProbability >= 0.4 && crossoverProbability <= 0.7))
            throw invalid_argument("crossoverProbability must be in [0, 1].");
        if (!(mutationProbability >= 0.001 && mutationProbability <= 0.1))
            throw invalid_argument("mutationProbability must be in [0.001, 0.1].");
    }
};

}
