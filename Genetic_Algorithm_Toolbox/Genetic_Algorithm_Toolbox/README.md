# Genetic Algorithm Toolbox and CVRP Solver

Soft Computing, Assignment 1. We built a genetic algorithm (GA) toolbox in C++17 without any GA
library, and then used it to solve the Capacitated Vehicle Routing Problem (CVRP).

## Folders

| Folder | What is inside |
|---|---|
| `GA_Core/` | The toolbox: GA loop, chromosomes, selection, crossover, mutation, replacement, and the CVRP parts |
| `app/` | `cvrp_solver.cpp`, the console program that solves a CVRP problem you type in |
| `tests/` | One test file for each team member |
| `experiments/` | Test problems, an exact solver for small problems, the experiment program and `RESULTS.md` |

## How to build and run

```
cmake -S . -B build
cmake --build build
cd build && ctest --output-on-failure && cd ..
```

This builds everything and runs the five test programs (`member1_tests` to `member5_tests`).
The tests also work in a Release build.

Run the CVRP solver:

```
./build/cvrp_solver
```

Run the experiments (use a Release build, it is much faster):

```
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target cvrp_experiments
./build-release/cvrp_experiments
```

## Part 1: The GA toolbox

The GA is a template on the chromosome type, so the same code works for any problem. You give it:

- a **fitness function** (higher is better, the GA maximizes),
- a **feasibility function** (true if the chromosome is valid),
- an **infeasibility handler** (what fitness an invalid chromosome gets),
- one selection, one crossover, one mutation and one replacement method.

### Components

| Part | What we have | File |
|---|---|---|
| Chromosomes | Binary, Integer, Floating-point | `BinaryChromosome`, `IntegerChromosome`, `FloatChromosome` |
| Selection | Tournament, Roulette wheel | `TournamentSelection.h`, `RouletteWheelSelection.h` |
| Crossover | Multi-point, Order (OX) | `MultiPointCrossover.h`, `OrderCrossover.h` |
| Mutation | Bit flip (binary), Swap, Inversion, Gaussian (float) | `BitFlipMutation.h`, `SwapMutation.h`, `InversionMutation.h`, `GaussianMutation.h` |
| Replacement | Generational, Elitist | `Replacement.h` |
| GA loop | Main loop and result | `GeneticAlgorithm.h`, `GAConfig.h`, `Interfaces.h` |

### How one generation works

```
pick 2 parents with the selection method
if random <= Pc: crossover, else copy the parents
mutate each child with Pm
check feasibility, then compute fitness (or ask the infeasibility handler)
when there are enough children: replacement makes the next population
```

The random number is drawn from [0, 1]. The best solution of the whole run is saved, with the
generation where it was found.

- **Generational replacement:** the children replace the whole population.
- **Elitist replacement:** the best individual stays, and the best children fill the other places,
  so the best solution is never lost.
- **Roulette wheel** subtracts the smallest fitness first, so it also works with negative fitness.
- **Mutation:** Bit flip, Swap and Gaussian check Pm for every gene. Inversion checks Pm once for
  the whole chromosome.

### Settings (`GAConfig`)

| Setting | Default | Allowed |
|---|---|---|
| `populationSize` | 100 | more than 0 |
| `maxGenerations` | 500 | more than 0 |
| `crossoverProbability` (Pc) | 0.6 | 0.4 to 0.7 |
| `mutationProbability` (Pm) | 0.01 | 0.001 to 0.1 |
| `seed` | 42 | any number |

The same seed and settings always give the same result.

## Part 2: CVRP

### Representation and decoding

Node 0 is the depot and nodes 1 to N are the customers. A chromosome is an `IntegerChromosome`
holding an order of all customers, for example `[3, 1, 4, 2]`.

The decoder reads the order from left to right and adds each customer to the current route. If
the next customer does not fit in the truck, it starts a new route. Every route starts and ends at
the depot.

To keep chromosomes valid, we use Order Crossover with Swap or Inversion mutation. The wrappers
in `CVRPPermutation.h` check after every crossover and mutation that the chromosome is still an
order of all customers, and stop with an error if it is not (for example multi-point crossover
breaks this, so it is not used for CVRP).

### Feasibility

A solution is feasible when every customer is visited exactly once, no truck is over capacity,
and it does not use more trucks than we have. `CVRPFeasibility` checks this.

### Fitness

The total distance D adds every edge of every route, including the trip back to the depot.

| Solution | Fitness |
|---|---|
| feasible | 1 / (1 + D), so a shorter distance gives a higher fitness |
| infeasible | -V, where V counts the problems (missing or repeated customers, extra trucks, ...) |

A feasible fitness is always above 0 and an infeasible one is always -1 or less, so every
feasible solution is ranked above every infeasible one. We do not need to tune a penalty value.
The `+ 1` stops a division by zero when D = 0.

### The console solver

The menu has **1. Solve a CVRP problem** and **2. Exit**. Option 1 asks for:

- the number of customers, the number of trucks and the truck capacity,
- the depot node (any node can be the depot),
- the demand of every node (0 for the depot),
- a distance matrix, or x y coordinates (straight-line distance).

Wrong values get an error message and you type them again. The GA runs 5 times (population 200,
500 generations, Tournament(3), Order Crossover, Swap, Elitist) and the best answer is printed in
the same format as the Google OR-Tools CVRP example:

```
Objective: 6208
Route for vehicle 0:
  0 Load(0) -> 9 Load(0) -> 14 Load(1) -> 16 Load(5) -> 10 Load(13) -> 0 Load(15)
Distance of the route: 1552m
Load of the route: 15

...
Total Distance of all routes: 6208m
Total Load of all routes: 60
```

`Load(x)` is the load of the customers visited before that node, and the last depot shows the
full load of the route. Whole-number distances have no decimals, other distances have 2.

On the 16-customer example from the OR-Tools guide
(https://developers.google.com/optimization/routing/cvrp) our solver finds a total distance of
6208, the same as OR-Tools. We only used the example data, not OR-Tools itself.



## Experiments

We tested the GA on four problems:

| Problem | Customers | Total demand | Trucks x capacity | Best answer |
|---|---|---|---|---|
| A-8 | 8 | 36 | 3 x 15 | 376.0714 (exact) |
| B-9 | 9 | 42 | 3 x 15 | 382.3667 (exact) |
| C-9-tight | 9 | 45 | 3 x 15, every truck full | 350.5129 (exact) |
| D-25 | 25 | 121 | 6 x 30 | 646.0235 (best we found) |

For problems with 10 customers or fewer, `CVRPExactSolver.h` finds the true best answer by trying
every order of the customers and every way to cut it into routes.

We changed one setting at a time from the defaults and ran each one 30 times. Some results:

| Setting | A-8 hits / gap | B-9 hits / gap | C-9 hits | D-25 gap |
|---|---|---|---|---|
| defaults | 22/30, 0.76% | 29/30, 0.18% | 30/30 | 13.02% |
| population 200 | 27/30, 0.20% | 30/30, 0.00% | 30/30 | 8.12% |
| Pm 0.1 | 29/30, 0.07% | 30/30, 0.00% | 30/30 | 23.58% |
| Roulette selection | 27/30, 0.20% | 30/30, 0.00% | 30/30 | 7.52% |

"Hits" means the run found the best answer, and "gap" is how much longer the answer was on
average. All 1560 runs gave a valid solution. The full tables and what we learned from them are
in `experiments/RESULTS.md`.


## Team

| Member                   | Part |
|--------------------------|---|
| Nancy - ID : 20230536    | GA loop, settings and replacement |
| Zeinab - ID : 20230165   | Chromosomes and selection |
| Esraa - ID : 20230052    | Crossover and mutation (with Member 2) |
| Abdallah - ID : 20230635 | CVRP representation, decoder and feasibility |
| Rawda - ID : 20230562    | CVRP fitness, integration, solver and experiments |
