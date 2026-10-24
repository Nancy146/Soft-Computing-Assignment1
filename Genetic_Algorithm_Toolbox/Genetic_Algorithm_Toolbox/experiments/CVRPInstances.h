#pragma once

#include "CVRPEvaluator.h"
#include "CVRPProblem.h"

#include <string>
#include <utility>
#include <vector>

namespace cvrp_instances {

using namespace std;
using namespace ga;

struct Instance {
    string name;
    vector<pair<double, double>> coordinates;
    CVRPProblem problem;

    DistanceMatrix distances() const { return euclideanDistances(coordinates); }
};

inline Instance a8() {
    return {"A-8",
            {{50, 50}, {20, 80}, {35, 90}, {75, 85}, {90, 60}, {80, 20}, {55, 10}, {15, 30}, {30, 55}},
            CVRPProblem({0, 4, 6, 3, 5, 7, 2, 6, 3}, 15, 3)};
}

inline Instance b9() {
    return {"B-9",
            {{40, 40}, {10, 10}, {20, 70}, {45, 85}, {70, 75}, {85, 40}, {65, 10}, {30, 25}, {60, 50}, {5, 50}},
            CVRPProblem({0, 5, 4, 7, 3, 6, 5, 2, 4, 6}, 15, 3)};
}

inline Instance c9Tight() {
    return {"C-9-tight",
            {{50, 50}, {80, 80}, {85, 70}, {75, 90}, {20, 15}, {10, 25}, {25, 5}, {15, 85}, {25, 90}, {10, 75}},
            CVRPProblem({0, 6, 5, 4, 6, 5, 4, 6, 5, 4}, 15, 3)};
}

inline Instance d25() {
    return {"D-25",
            {{50, 50}, {90, 9}, {10, 10}, {44, 8}, {24, 37}, {81, 84}, {95, 69}, {28, 15}, {78, 51},
             {0, 9}, {3, 7}, {23, 67}, {65, 44}, {18, 8}, {67, 71}, {74, 89}, {15, 17}, {52, 9},
             {15, 26}, {28, 30}, {99, 1}, {23, 47}, {80, 14}, {44, 98}, {65, 46}, {50, 58}},
            CVRPProblem({0, 2, 1, 4, 6, 7, 5, 8, 8, 7, 6, 4, 8, 3, 3, 2, 6, 6, 9, 4, 4, 4, 5, 3, 1, 5}, 30, 6)};
}

inline vector<Instance> all() { return {a8(), b9(), c9Tight(), d25()}; }

}
