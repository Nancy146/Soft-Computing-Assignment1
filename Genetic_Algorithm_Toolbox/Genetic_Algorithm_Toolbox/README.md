# Member 1 — GA Core, Architecture & Replacement

Header-only C++17 GA core. It is a template on the chromosome type, so it does not know
how chromosomes are represented. Chromosome types are Member 2's responsibility.

## What is here

| File | Content |
|---|---|
| `GA_Core/GAConfig.h` | Hyperparameters with defaults + `validate()` |
| `GA_Core/Interfaces.h` | Contracts with the other members (see below) |
| `GA_Core/Replacement.h` | `GenerationalReplacement`, `ElitistReplacement` |
| `GA_Core/GeneticAlgorithm.h` | Main GA loop + `GAResult` |
| `tests/test_member1.cpp` | Unit tests for the Member 1 components only |

Intentionally **not** here: chromosome types, initialization and validation (Member 2),
selection (Member 2), crossover and mutation (Members 2 & 3), CVRP representation,
feasibility, and infeasibility handling (Member 4), CVRP fitness (Member 5).

## Main loop ("Canonical Genetic Algorithm")

```
Initial population (from Member 2's initialization)
  -> evaluate fitness
  -> repeat maxGenerations times:
       select 2 parents
       random < Pc ?  crossover  :  copy the parents
       mutate each child with Pm
       evaluate the offspring
       replacement  (returns survivors + their fitness, so nothing is re-evaluated)
  -> return the best individual found
```

Fitness is computed exactly `populationSize × (maxGenerations + 1)` times.

## Interfaces (C = chromosome type)

| Interface | Owner | Contract |
|---|---|---|
| `SelectionStrategy<C>::select(population, fitness, rng) -> size_t` | Member 2 | Returns the **index** of one parent |
| `CrossoverStrategy<C>::crossover(p1, p2, rng) -> pair<C, C>` | Member 3 | **Always** performs crossover (the core applies Pc). Must not modify the parents |
| `MutationStrategy<C>::mutate(c, probability, rng)` | Members 2 & 3 | Mutates `c` in place. `probability` = Pm |
| `FitnessFunction<C>` = `function<double(const C&)>` | Member 5 | **Higher is better** (the GA maximizes) |
| `FeasibilityFunction<C>` = `function<bool(const C&)>` | Member 4 | `true` if the chromosome satisfies the constraints |
| `InfeasibilityHandler<C>::handle(c, fitness, feasibility, rng) -> double` | Member 4 | Called **only** for infeasible chromosomes. Returns the fitness to use (penalty, repair, ...). May modify `c` (repair) |
| `ReplacementStrategy<C>::replace(...) -> ReplacementResult<C>` | Member 1 | Returns the next population **and** its fitness, same size as the current population |

## Replacement methods

- **Generational:** the offspring replace the entire old population.
- **Elitist:** copy the best individual of the current population, then fill the remaining
  `populationSize - 1` positions with the best offspring. The best solution is never lost.

## Hyperparameters (`GAConfig`)

| Field | Default | Valid range |
|---|---|---|
| `populationSize` | 100 | > 0 |
| `maxGenerations` | 500 | > 0 |
| `crossoverProbability` (Pc) | 0.8 | [0, 1] |
| `mutationProbability` (Pm) | 0.1 | [0, 1] |
| `seed` | 42 | any |

The same seed, configuration, and problem always give the same result. All randomness must
come from the `rng` passed to each interface, never from `rand()` or another generator.

## Result (`GAResult<C>`)

`bestChromosome`, `bestFitness`, `bestGeneration`, `bestIsFeasible`, and per generation
(index 0 = initial population) `bestHistory` and `averageHistory` for Member 5's
convergence analysis.

## To confirm with the team

1. **Pm per gene or per chromosome.** The lecture checks Pm independently for each gene, so
   that is the current convention. Members 2 & 3 must implement mutation the same way.
2. **Generation info for mutation.** If the team picks non-uniform mutation, `mutate` needs the
   current generation `t` and `maxGenerations` (`T`). Add a `GAContext` to the signature
   **before** Members 2 & 3 write their mutations.
3. **Positive fitness.** Roulette-wheel selection needs fitness > 0. If the team uses it,
   the CVRP fitness must be positive, for example `1 / (distance + penalty)`.
4. **CVRP representation** (Members 2, 3, 4): which chromosome type CVRP uses, and how
   Order Crossover keeps it valid.

## Build and test

```
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Or without CMake:

```
g++ -std=c++17 -Wall -Wextra -IGA_Core tests/test_member1.cpp -o member1_tests && ./member1_tests
```
