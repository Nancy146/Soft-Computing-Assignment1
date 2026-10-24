#pragma once

#include "CVRPDecoder.h"
#include <vector>
using namespace std;

namespace ga {
class CVRPFeasibility {
public:
    explicit CVRPFeasibility(const CVRPProblem& problem) : problem_(problem) {}

    bool operator()(const IntegerChromosome& chromosome) const {
        return CVRPDecoder::decodeAndCheck(chromosome, problem_).feasible();
    }

private:
    CVRPProblem problem_;
};

}
