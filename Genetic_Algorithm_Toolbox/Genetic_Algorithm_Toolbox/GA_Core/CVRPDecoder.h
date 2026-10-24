#pragma once

#include "CVRPProblem.h"
#include "IntegerChromosome.h"
#include <vector>
using namespace std;

namespace ga {
struct CVRPDecodedSolution {
    vector<vector<int>> routes;
    vector<int> routeLoads;
    int invalidGenes = 0;
    int duplicateCustomers = 0;
    int missingCustomers = 0;
    int capacityExcess = 0;
    int extraVehicles = 0;

    int totalViolation() const {
        return invalidGenes + duplicateCustomers + missingCustomers + capacityExcess + extraVehicles;
    }
    bool feasible() const { return totalViolation() == 0; }
};

class CVRPDecoder {
public:
    using Route = vector<int>;
    using Routes = vector<Route>;

    static Routes decode(const IntegerChromosome& chromosome, const CVRPProblem& problem)
    {
        return decodeAndCheck(chromosome, problem).routes;
    }

    static CVRPDecodedSolution decodeAndCheck(const IntegerChromosome& chromosome, const CVRPProblem& problem)
    {
        CVRPDecodedSolution solution;
        vector<bool> seen(problem.customerCount() + 1, false);
        Route currentRoute;
        int currentLoad = 0;

        auto closeRoute = [&] {
            solution.routes.push_back(currentRoute);
            solution.routeLoads.push_back(currentLoad);
            if (currentLoad > problem.vehicleCapacity)
                solution.capacityExcess += currentLoad - problem.vehicleCapacity;
            currentRoute.clear();
            currentLoad = 0;
        };

        for (int customer : chromosome.getGenes()) {
            if (customer < 1 || static_cast<size_t>(customer) > problem.customerCount()) {
                ++solution.invalidGenes;
                continue;
            }
            const size_t id = static_cast<size_t>(customer);
            if (seen[id])
                ++solution.duplicateCustomers;
            seen[id] = true;

            const int demand = problem.demands[id];
            if (!currentRoute.empty() && currentLoad + demand > problem.vehicleCapacity)
                closeRoute();

            currentRoute.push_back(customer);
            currentLoad += demand;
        }

        if (!currentRoute.empty())
            closeRoute();

        for (size_t id = 1; id < seen.size(); ++id) {
            if (!seen[id])
                ++solution.missingCustomers;
        }
        if (solution.routes.size() > static_cast<size_t>(problem.vehicleCount))
            solution.extraVehicles = static_cast<int>(solution.routes.size()) - problem.vehicleCount;

        return solution;
    }
};

}
