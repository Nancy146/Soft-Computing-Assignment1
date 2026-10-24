# Experiment results

We ran our GA on four CVRP problems to see how good its answers are and how each setting
changes the result.

## How we tested

- We started from one **baseline** setting: population 100, 500 generations, Pc = 0.6,
  Pm = 0.01, Tournament selection (size 3), Order Crossover, Swap mutation and Elitist replacement.
- Then we changed **one thing at a time** (population, generations, Pc, Pm, selection, mutation
  or replacement) and kept everything else the same as the baseline.
- Every setting was run **30 times** with seeds 1 to 30, on every problem.
- For the three small problems (8 and 9 customers) we know the **best possible answer**,
  because our exact solver tries every order of the customers. For the big problem (25 customers)
  that is too slow, so we compare with the best answer any of our runs found.

## What the columns mean

- **Feasible**: how many of the 30 runs gave a valid solution (no truck over capacity and not
  more trucks than we have).
- **Hits**: how many runs found the best answer.
- **Mean gap %**: on average, how much longer our total distance was than the best answer.
  0 means we found the best answer every time.
- **Worst gap %**: the gap of the worst run.
- **Best gen**: on average, in which generation the final answer was found.
- **Time ms**: the average time of one run in milliseconds. This depends on the computer, so it
  changes a little every time. All the other columns are always the same because the seeds are fixed.

## Results

### A-8: 8 customers, 3 trucks, capacity 15

Best possible distance: **376.0714**.

| Factor | Setting | Feasible | Hits | Mean gap % | Worst gap % | Best gen | Time ms |
|---|---|---|---|---|---|---|---|
| baseline | defaults | 30/30 | 22/30 | 0.76 | 4.71 | 4 | 43.3 |
| population | 50 | 30/30 | 15/30 | 2.13 | 9.81 | 19 | 21.7 |
| population | 200 | 30/30 | 27/30 | 0.20 | 1.99 | 4 | 85.8 |
| generations | 100 | 30/30 | 22/30 | 0.76 | 4.71 | 4 | 8.7 |
| generations | 1000 | 30/30 | 22/30 | 0.73 | 4.71 | 32 | 86.6 |
| Pc | 0.4 | 30/30 | 14/30 | 1.61 | 6.68 | 38 | 42.3 |
| Pc | 0.7 | 30/30 | 18/30 | 1.08 | 4.71 | 5 | 42.9 |
| Pm | 0.001 | 30/30 | 24/30 | 0.50 | 3.56 | 5 | 42.0 |
| Pm | 0.05 | 30/30 | 24/30 | 0.40 | 1.99 | 17 | 43.5 |
| Pm | 0.1 | 30/30 | 29/30 | 0.07 | 1.99 | 14 | 45.2 |
| selection | Roulette | 30/30 | 27/30 | 0.20 | 1.99 | 30 | 51.2 |
| mutation | Inversion | 30/30 | 21/30 | 0.60 | 1.99 | 5 | 32.8 |
| replacement | Generational | 30/30 | 23/30 | 0.61 | 4.71 | 5 | 41.4 |

### B-9: 9 customers, 3 trucks, capacity 15

Best possible distance: **382.3667**.

| Factor | Setting | Feasible | Hits | Mean gap % | Worst gap % | Best gen | Time ms |
|---|---|---|---|---|---|---|---|
| baseline | defaults | 30/30 | 29/30 | 0.18 | 5.25 | 8 | 45.5 |
| population | 50 | 30/30 | 27/30 | 0.53 | 5.25 | 11 | 22.4 |
| population | 200 | 30/30 | 30/30 | 0.00 | 0.00 | 5 | 90.8 |
| generations | 100 | 30/30 | 29/30 | 0.18 | 5.25 | 8 | 9.2 |
| generations | 1000 | 30/30 | 29/30 | 0.18 | 5.25 | 8 | 91.0 |
| Pc | 0.4 | 30/30 | 30/30 | 0.00 | 0.00 | 9 | 44.4 |
| Pc | 0.7 | 30/30 | 27/30 | 0.53 | 5.25 | 6 | 46.0 |
| Pm | 0.001 | 30/30 | 26/30 | 0.70 | 5.25 | 8 | 44.5 |
| Pm | 0.05 | 30/30 | 29/30 | 0.18 | 5.25 | 9 | 47.0 |
| Pm | 0.1 | 30/30 | 30/30 | 0.00 | 0.00 | 9 | 49.4 |
| selection | Roulette | 30/30 | 30/30 | 0.00 | 0.00 | 34 | 56.0 |
| mutation | Inversion | 30/30 | 29/30 | 0.18 | 5.25 | 21 | 34.6 |
| replacement | Generational | 30/30 | 28/30 | 0.35 | 5.25 | 8 | 44.3 |

### C-9-tight: 9 customers, 3 trucks, capacity 15

Best possible distance: **350.5129**. The total demand is 45 = 3 x 15, so every truck must be completely full.

| Factor | Setting | Feasible | Hits | Mean gap % | Worst gap % | Best gen | Time ms |
|---|---|---|---|---|---|---|---|
| baseline | defaults | 30/30 | 30/30 | 0.00 | 0.00 | 9 | 45.9 |
| population | 50 | 30/30 | 30/30 | 0.00 | 0.00 | 16 | 22.5 |
| population | 200 | 30/30 | 30/30 | 0.00 | 0.00 | 7 | 90.6 |
| generations | 100 | 30/30 | 30/30 | 0.00 | 0.00 | 9 | 9.1 |
| generations | 1000 | 30/30 | 30/30 | 0.00 | 0.00 | 9 | 90.3 |
| Pc | 0.4 | 30/30 | 30/30 | 0.00 | 0.00 | 9 | 44.3 |
| Pc | 0.7 | 30/30 | 30/30 | 0.00 | 0.00 | 8 | 46.1 |
| Pm | 0.001 | 30/30 | 30/30 | 0.00 | 0.00 | 19 | 45.0 |
| Pm | 0.05 | 30/30 | 30/30 | 0.00 | 0.00 | 7 | 47.4 |
| Pm | 0.1 | 30/30 | 30/30 | 0.00 | 0.00 | 17 | 50.2 |
| selection | Roulette | 30/30 | 30/30 | 0.00 | 0.00 | 32 | 55.8 |
| mutation | Inversion | 30/30 | 30/30 | 0.00 | 0.00 | 25 | 34.6 |
| replacement | Generational | 30/30 | 30/30 | 0.00 | 0.00 | 9 | 45.0 |

### D-25: 25 customers, 6 trucks, capacity 30

Best distance found by any run: **646.0235** (this may not be the true best answer).

| Factor | Setting | Feasible | Hits | Mean gap % | Worst gap % | Best gen | Time ms |
|---|---|---|---|---|---|---|---|
| baseline | defaults | 30/30 | 0/30 | 13.02 | 27.49 | 163 | 88.7 |
| population | 50 | 30/30 | 0/30 | 14.39 | 33.16 | 214 | 44.9 |
| population | 200 | 30/30 | 1/30 | 8.12 | 15.87 | 167 | 178.0 |
| generations | 100 | 30/30 | 0/30 | 14.35 | 30.26 | 85 | 18.0 |
| generations | 1000 | 30/30 | 0/30 | 12.19 | 27.32 | 371 | 177.2 |
| Pc | 0.4 | 30/30 | 0/30 | 11.20 | 24.03 | 149 | 86.6 |
| Pc | 0.7 | 30/30 | 0/30 | 10.75 | 20.70 | 167 | 90.2 |
| Pm | 0.001 | 30/30 | 0/30 | 16.07 | 27.45 | 348 | 89.0 |
| Pm | 0.05 | 30/30 | 0/30 | 11.97 | 23.92 | 432 | 95.9 |
| Pm | 0.1 | 30/30 | 0/30 | 23.58 | 36.46 | 428 | 98.3 |
| selection | Roulette | 30/30 | 0/30 | 7.52 | 16.89 | 368 | 100.9 |
| mutation | Inversion | 30/30 | 0/30 | 14.80 | 32.35 | 433 | 56.7 |
| replacement | Generational | 30/30 | 0/30 | 11.31 | 25.79 | 175 | 88.9 |

## What we noticed

- **Every run gave a valid solution.** All 1560 runs ended with no truck over capacity, even on
  C-9-tight where every truck has to be full.
- **The small problems are easy.** With the baseline, the GA found the best answer in 22/30 runs
  on A-8, 29/30 on B-9 and 30/30 on C-9-tight. Even the worst runs were within about 5% of the best.
- **The big problem is harder.** On D-25 the baseline is 13.02% longer than the best answer on
  average, so the GA gives a good answer but not always the best one.
- **A bigger population helps.** Population 200 gave better answers on A-8, B-9 and D-25 (D-25
  gap went from 13.02% to 8.12%; C-9-tight was already solved every time), but each run takes about twice as long.
- **More generations did not help much.** The GA finds its answer early (generation 4 to 9 on the
  small problems), so 1000 generations were almost the same as 500 (D-25: 12.19% vs 13.02%).
- **Pm must not be too big.** Pm = 0.1 was the best on the small problems but the worst on D-25
  (23.58%). Swap mutation uses Pm for every gene, so with 25 genes a child gets about 2.5 swaps,
  and that breaks good routes. Pm = 0.01 is a safe choice for all four problems.
- **Roulette selection did well on D-25** (7.52%, the lowest gap), but it found its answer later
  (generation 368 instead of 163). We think this is because our fitness values are close to each
  other, so roulette keeps more variety in the population. We did not test this separately.
- **Pc and replacement made small changes.** Pc = 0.7 was better on D-25 but worse on A-8, and
  Generational replacement stayed close to the baseline.
- **Swap and Inversion do not use Pm the same way.** Swap checks Pm for every gene, but Inversion
  checks it only once per chromosome. With Pm = 0.01 and 25 customers, Swap changes a child about
  0.25 times on average and Inversion only about 0.01 times. So the Inversion row is not only a
  different operator, it also has much less mutation.

## Limits of these tests

- We changed one setting at a time, so we did not test what happens when two settings change together.
- 30 runs is not very many, so small differences on A-8 and B-9 could be luck.
- For D-25 we do not know the true best answer.

## How to run it again

```
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target cvrp_experiments
./build-release/cvrp_experiments
```

The program prints the tables above.
