#pragma once

#include <stdexcept>
#include <utility>
#include <vector>
using namespace std;

namespace ga {

struct CVRPProblem {
    vector<int> demands;
    int vehicleCapacity = 0;
    int vehicleCount = 0;

    CVRPProblem(vector<int> customerDemands,int capacity,int vehicles): demands(std::move(customerDemands)), vehicleCapacity(capacity), vehicleCount(vehicles)
    {
        if (demands.size() < 2)
            throw invalid_argument("CVRP requires at least one customer; demands[0] is the depot.");
        if (demands[0] != 0)
            throw invalid_argument("demands[0] must be 0 for the depot.");
        if (vehicleCapacity <= 0)
            throw invalid_argument("Vehicle capacity must be positive.");
        if (vehicleCount <= 0)
            throw invalid_argument("Vehicle count must be positive.");
        for (size_t customer = 1; customer < demands.size(); ++customer) {
            if (demands[customer] < 0)
                throw invalid_argument("Customer demands cannot be negative.");
        }
    }

    size_t customerCount() const noexcept { return demands.size() - 1; }
};

}
