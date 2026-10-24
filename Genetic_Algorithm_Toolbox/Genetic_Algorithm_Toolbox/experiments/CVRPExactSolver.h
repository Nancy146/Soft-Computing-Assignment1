#pragma once

#include "CVRPDecoder.h"
#include "CVRPEvaluator.h"
#include "CVRPProblem.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace cvrp_exact {

struct Solution {
    bool feasible = false;
    double distance = 1e18;
    ga::CVRPDecoder::Routes routes;
};

inline Solution solve(const ga::CVRPProblem& problem, const ga::DistanceMatrix& d) {
    int n = static_cast<int>(problem.customerCount());
    int maxRoutes = problem.vehicleCount;
    if (n > 10)
        throw std::invalid_argument("cvrp_exact::solve: at most 10 customers.");

    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 1);
    Solution best;

    do {
        std::vector<std::vector<double>> cost(maxRoutes + 1, std::vector<double>(n + 1, 1e18));
        std::vector<std::vector<int>> cut(maxRoutes + 1, std::vector<int>(n + 1, 0));
        cost[0][0] = 0;

        for (int k = 1; k <= maxRoutes; k++) {
            for (int start = 0; start < n; start++) {
                if (cost[k - 1][start] >= 1e18)
                    continue;
                int load = 0;
                double length = 0;
                for (int end = start; end < n; end++) {
                    int customer = order[end];
                    load += problem.demands[customer];
                    if (load > problem.vehicleCapacity)
                        break;
                    if (end == start)
                        length += d[0][customer];
                    else
                        length += d[order[end - 1]][customer];
                    double total = cost[k - 1][start] + length + d[customer][0];
                    if (total < cost[k][end + 1]) {
                        cost[k][end + 1] = total;
                        cut[k][end + 1] = start;
                    }
                }
            }
        }

        for (int k = 1; k <= maxRoutes; k++) {
            if (cost[k][n] < best.distance) {
                best.feasible = true;
                best.distance = cost[k][n];
                best.routes.clear();
                int end = n;
                for (int r = k; r > 0; r--) {
                    int start = cut[r][end];
                    best.routes.insert(best.routes.begin(),
                                       ga::CVRPDecoder::Route(order.begin() + start, order.begin() + end));
                    end = start;
                }
            }
        }
    } while (std::next_permutation(order.begin(), order.end()));

    return best;
}

}
