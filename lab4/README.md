# CS360 Lab 4

Multi-threaded Linear Algebra: LU Decomposition and Gaussian Elimination

## Running the Program

Compile the project by running `make`. This will compile the program into the `bin` directory.

From there, `cd` into the `bin` directory and run `./lu.bin` or `./pivot.bin`.

- **lu.bin** (`lu.c`): Solves A*X = B using LU decomposition with partial pivoting. Produces separate P, L, U matrices then applies forward/backward substitution.
- **pivot.bin** (`pivot.c`): Solves an augmented system [A|B] using Gaussian elimination with partial pivoting. The solution is computed inline via back-substitution.

## Key Concepts

### LU Decomposition: A = P·L·U

LU decomposition factors a matrix A into three matrices:

| Matrix | Description |
|--------|-------------|
| P | Permutation — records which rows were swapped during pivoting |
| L | Lower triangular with 1s on diagonal — the elimination multipliers |
| U | Upper triangular — the result after elimination |

To solve A·X = B with the factored form:
1. Permute B: `b = P·B`
2. Forward substitution: solve `L·Y = b`
3. Backward substitution: solve `U·X = Y`

### Partial Pivoting (Numerical Stability)

Before eliminating column k, we find the row with the **largest absolute value** in that column and swap it to the pivot position. This prevents division by near-zero values which would amplify floating-point errors.

### Barrier Synchronization

`pthread_barrier_wait()` blocks a thread until all N threads have called it. This replaces explicit locks for this pattern:

```
Thread 0  [do step 0] [BARRIER] [parallel work] [BARRIER]
Thread 1  [BARRIER]   [BARRIER] [parallel work] [BARRIER]
...
Thread N-1[BARRIER]   [BARRIER] [parallel work] [BARRIER]
```

Each thread "owns" one step (thread k handles pivot step k). All threads synchronize between ownership sections so every thread sees consistent shared state.

### Forward and Backward Substitution

Once L and U are known, solving is cheap:

**Forward** (L·Y = b): L is lower triangular with 1s on diagonal, so:
```
Y[i] = b[i] - sum(L[i][j] * Y[j] for j < i)
```

**Backward** (U·X = Y): U is upper triangular, solve bottom-to-top:
```
X[i] = (Y[i] - sum(U[i][j] * X[j] for j > i)) / U[i][i]
```

## Authors

* **Connor Easton**  - [Eastonco](https://github.com/Eastonco)
* **KC Wang**  - [KC Wang](https://school.eecs.wsu.edu/faculty/profile/?nid=kwang)
