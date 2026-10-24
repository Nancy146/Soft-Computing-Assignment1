#include "CVRPDecoder.h"
#include "CVRPEvaluator.h"
#include "CVRPFeasibility.h"
#include "CVRPPenaltyHandler.h"
#include "CVRPPermutation.h"
#include "CVRPProblem.h"
#include "GeneticAlgorithm.h"
#include "OrderCrossover.h"
#include "Replacement.h"
#include "SwapMutation.h"
#include "TournamentSelection.h"

#include <cmath>
#include <cstddef>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

using namespace std;
using namespace ga;

constexpr size_t populationSize = 200;
constexpr size_t generations = 500;
constexpr unsigned int runs = 5;

constexpr long long maxCustomers = 500;
constexpr long long maxVehicles = 1000;
constexpr long long maxCapacity = 1000000;

struct EndOfInput {};

bool parseValue(const string& text, bool whole, double& value) {
    try {
        size_t used = 0;
        value = whole ? static_cast<double>(stoll(text, &used)) : stod(text, &used);
        return used == text.size() && isfinite(value);
    } catch (const exception&) {
        return false;
    }
}

vector<double> readValues(const string& prompt, size_t count, bool whole, const function<string(size_t, double)>& check) {
    for (;;) {
        cout << prompt << flush;
        vector<double> values;
        string error;
        while (values.size() < count && error.empty()) {
            string token;
            if (!(cin >> token))
                throw EndOfInput{};
            double value = 0.0;
            if (!parseValue(token, whole, value))
                error = "'" + token + "' is not " + (whole ? "a whole number." : "a number.");
            else
                error = check(values.size(), value);
            if (error.empty())
                values.push_back(value);
        }
        if (error.empty())
            return values;
        cout << "Error: " << error << (count == 1 ? " Please try again.\n" : " Please enter these values again.\n");
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

long long readInteger(const string& prompt, long long low, long long high, const string& what) {
    return static_cast<long long>(readValues(prompt, 1, true, [&](size_t, double value) -> string {
        if (value < low || value > high)
            return what + " must be between " + to_string(low) + " and " + to_string(high) + ".";
        return "";
    })[0]);
}

struct UserProblem {
    size_t nodes = 0;
    size_t depot = 0;
    int vehicles = 0;
    int capacity = 0;
    vector<int> demands;
    DistanceMatrix distances;
};

bool readProblem(UserProblem& problem) {
    const long long customers = readInteger("Number of customers: ", 1, maxCustomers, "The number of customers");
    const long long vehicles = readInteger("Number of vehicles: ", 1, maxVehicles, "The number of vehicles");
    const long long capacity = readInteger("Vehicle capacity: ", 1, maxCapacity, "The vehicle capacity");
    const size_t nodes = static_cast<size_t>(customers) + 1;
    const long long depot = readInteger("Depot node (0-" + to_string(customers) + "): ", 0, customers, "The depot node");

    const vector<double> demands = readValues(
        "Demands of nodes 0-" + to_string(customers) + " (" + to_string(nodes) + " whole numbers; the depot's demand is 0):\n",
        nodes, true, [&](size_t node, double value) -> string {
            if (value > static_cast<double>(maxCapacity))
                return "node " + to_string(node) + " has a demand above " + to_string(maxCapacity) +
                       ", more than any vehicle capacity, so no vehicle can serve it.";
            const long long demand = static_cast<long long>(value);
            if (demand < 0)
                return "node " + to_string(node) + " has a negative demand (" + to_string(demand) + ").";
            if (node == static_cast<size_t>(depot) && demand != 0)
                return "the depot (node " + to_string(depot) + ") must have demand 0, not " + to_string(demand) + ".";
            if (demand > capacity)
                return "node " + to_string(node) + " has demand " + to_string(demand) + ", more than the vehicle capacity " +
                       to_string(capacity) + ", so no vehicle can serve it.";
            return "";
        });
    long long totalDemand = 0;
    for (double demand : demands)
        totalDemand += static_cast<long long>(demand);
    if (totalDemand > vehicles * capacity) {
        cout << "Error: the total demand " << totalDemand << " is more than the fleet can carry (" << vehicles << " vehicles x "
             << capacity << " = " << vehicles * capacity << "), so no solution exists. Please enter the problem again.\n\n";
        return false;
    }

    const long long mode =
        readInteger("Distance input (1 = distance matrix, 2 = coordinates, Euclidean distance): ", 1, 2, "The distance input");
    DistanceMatrix distances(nodes);
    if (mode == 1) {
        cout << "Distance matrix: " << nodes << " rows of " << nodes << " numbers (row i = distances from node i to nodes 0-"
             << customers << "):\n";
        for (size_t row = 0; row < nodes; ++row) {
            distances[row] = readValues("Row " + to_string(row) + ": ", nodes, false, [&](size_t column, double d) -> string {
                if (d < 0.0)
                    return "row " + to_string(row) + ", column " + to_string(column) + ": a distance cannot be negative.";
                if (column == row && d != 0.0)
                    return "row " + to_string(row) + ", column " + to_string(column) +
                           ": the distance from a node to itself must be 0.";
                return "";
            });
        }
    } else {
        cout << "Coordinates: one line \"x y\" per node 0-" << customers << ":\n";
        vector<pair<double, double>> coordinates;
        for (size_t node = 0; node < nodes; ++node) {
            const vector<double> xy = readValues("Node " + to_string(node) + ": ", 2, false, [](size_t, double) { return string(); });
            coordinates.emplace_back(xy[0], xy[1]);
        }
        distances = euclideanDistances(coordinates);
    }

    problem.nodes = nodes;
    problem.depot = static_cast<size_t>(depot);
    problem.vehicles = static_cast<int>(vehicles);
    problem.capacity = static_cast<int>(capacity);
    problem.demands.assign(demands.begin(), demands.end());
    problem.distances = std::move(distances);
    return true;
}

string formatNumber(double value) {
    ostringstream out;
    if (fabs(value) < 1e15 && fabs(value - round(value)) < 1e-9)
        out << llround(value);
    else
        out << fixed << setprecision(2) << value;
    return out.str();
}

CVRPEvaluation bestOfRuns(const CVRPEvaluator& evaluator) {
    const size_t customers = evaluator.problem().customerCount();
    CVRPEvaluation best;
    double bestScore = -numeric_limits<double>::infinity();
    for (unsigned int seed = 1; seed <= runs; ++seed) {
        GAConfig config;
        config.populationSize = populationSize;
        config.maxGenerations = generations;
        config.seed = seed;
        mt19937 initRng(seed + 1000);
        GeneticAlgorithm<IntegerChromosome> ga(
            config, randomCustomerPermutations(config.populationSize, customers, initRng),
            evaluator.fitnessFunction(), CVRPFeasibility(evaluator.problem()), make_shared<CVRPFeasibleFirstHandler>(evaluator),
            make_shared<TournamentSelection<IntegerChromosome>>(3),
            make_shared<PermutationCheckedCrossover>(make_shared<OrderCrossover<IntegerChromosome>>(), customers),
            make_shared<PermutationCheckedMutation>(make_shared<SwapMutation<IntegerChromosome>>(), customers),
            make_shared<ElitistReplacement<IntegerChromosome>>());
        const CVRPEvaluation evaluation = evaluator.evaluate(ga.run().bestChromosome);
        const double score = CVRPEvaluator::score(evaluation);
        if (score > bestScore) {
            bestScore = score;
            best = evaluation;
        }
    }
    return best;
}

void solveProblem(const UserProblem& user) {
    vector<size_t> userNode{user.depot};
    for (size_t node = 0; node < user.nodes; ++node)
        if (node != user.depot)
            userNode.push_back(node);

    vector<int> demands;
    DistanceMatrix distances(user.nodes, vector<double>(user.nodes, 0.0));
    for (size_t a = 0; a < user.nodes; ++a) {
        demands.push_back(user.demands[userNode[a]]);
        for (size_t b = 0; b < user.nodes; ++b)
            distances[a][b] = user.distances[userNode[a]][userNode[b]];
    }
    const CVRPEvaluator evaluator(CVRPProblem(demands, user.capacity, user.vehicles), distances);
    const CVRPEvaluation best = bestOfRuns(evaluator);
    const CVRPDecodedSolution& solution = best.solution;

    if (!best.feasible()) {
        cout << "No feasible solution found. The best solution found needs " << solution.routes.size() << " routes for "
             << user.vehicles << " vehicles (" << solution.extraVehicles << " too many";
        if (solution.capacityExcess > 0)
            cout << ", capacity exceeded by " << solution.capacityExcess;
        cout << ").\nThe demands may not fit into " << user.vehicles << " vehicles; try more vehicles or a larger capacity.\n";
        return;
    }

    cout << "Objective: " << formatNumber(best.totalDistance) << '\n';
    int totalLoad = 0;
    for (size_t vehicle = 0; vehicle < solution.routes.size(); ++vehicle) {
        int load = 0;
        cout << "Route for vehicle " << vehicle << ":\n";
        cout << "  " << userNode[0] << " Load(" << load << ")";
        for (int customer : solution.routes[vehicle]) {
            cout << " -> " << userNode[static_cast<size_t>(customer)] << " Load(" << load << ")";
            load += demands[static_cast<size_t>(customer)];
        }
        cout << " -> " << userNode[0] << " Load(" << load << ")\n";
        cout << "Distance of the route: " << formatNumber(evaluator.routeDistance(solution.routes[vehicle])) << "m\n";
        cout << "Load of the route: " << solution.routeLoads[vehicle] << "\n\n";
        totalLoad += solution.routeLoads[vehicle];
    }
    cout << "Total Distance of all routes: " << formatNumber(best.totalDistance) << "m\n";
    cout << "Total Load of all routes: " << totalLoad << '\n';
}

int main() {
    cout << "=== CVRP Solver (Genetic Algorithm Toolbox) ===\n"
         << "Nodes are numbered 0-N; one of them is the depot, the others are customers.\n";
    try {
        for (;;) {
            cout << "\n1. Solve a CVRP problem\n2. Exit\n";
            if (readInteger("Choose an option (1-2): ", 1, 2, "The option") == 2)
                break;
            UserProblem problem;
            while (!readProblem(problem)) {
            }
            cout << "\nSolving with the genetic algorithm (best of " << runs << " runs, population " << populationSize << ", "
                 << generations << " generations)...\n\n";
            try {
                solveProblem(problem);
            } catch (const exception& e) {
                cout << "Error while solving: " << e.what() << '\n';
            }
        }
    } catch (const EndOfInput&) {
        cout << "\nInput ended.\n";
    }
    return 0;
}
