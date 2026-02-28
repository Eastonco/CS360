/** lu.c: LU decomposition with Partial Pivoting using pthreads **/
/*
 * Algorithm: A = P * L * U  (LU decomposition with partial pivoting)
 *
 * We factor a matrix A into three matrices:
 *   P — permutation matrix (records row swaps done during pivoting)
 *   L — lower triangular with 1s on the diagonal
 *   U — upper triangular
 *
 * To solve A*X = B, we use the factored form P*L*U*X = B:
 *   1. Apply P to B → b  (permute B according to row swap history)
 *   2. Forward substitution:  solve L*Y = b
 *   3. Backward substitution: solve U*X = Y
 *
 * Threading model:
 *   N threads are created. In each step k (0..n-1), thread k "owns" that step:
 *   it does the partial pivoting, row swaps, and L/U entry computation.
 *   All other threads wait at barriers. After each ownership section, every
 *   thread participates in the parallel row reduction of A.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define N 8
int n;

double A[N][N], L[N][N], U[N][N];
double B[N], b[N];
int P[N];
double Y[N], X[N];
pthread_barrier_t barrier; /* synchronization point shared by all N threads */

/* BUG FIX: return type was int; print functions don't return values */
void print(char c, double x[N][N])
{
    int i, j;
    printf("------------- %c -----------------------\n", c);
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
            printf("%6.2f  ", x[i][j]);
        printf("\n");
    }
}

void printV(char c, double x[N])
{
    int i;
    printf("--------- %c vector-----------\n", c);
    for (i = 0; i < n; i++)
        printf("%6.2f ", x[i]);
    printf("\n");
}

void printP(void)
{
    int i;
    printf("--------- P vector-----------\n");
    for (i = 0; i < n; i++)
        printf("%d ", P[i]);
    printf("\n");
}

/*
 * lu: the pthread worker function.
 *
 * BUG FIX: return type was int* — pthread start routines must be void *(*)(void *).
 * Using the wrong type causes undefined behavior when pthread_create casts the pointer.
 *
 * Each thread receives its ID (0..N-1) via the arg pointer (cast from int).
 * Thread k "owns" step k: it performs partial pivoting and LU updates for column k.
 * Barriers synchronize all threads between each critical section.
 */
void *lu(void *arg)
{
    int i, j, k, m, itemp;
    double max;
    double temp;
    int myid = (int)(long)arg; /* cast via long to avoid pointer truncation warning */

    for (k = 0; k < n; k++)
    {
        /* STEP 1 (thread k): Find the row with the largest value in column k
         * at or below row k — "partial pivoting" to improve numerical stability. */
        if (k == myid)
        {
            max = 0;
            printf("partial pivoting by row %d on thread %d\n", k, myid);
            for (i = k; i < n; i++)
            {
                if (max < fabs(A[i][k]))
                {
                    max = fabs(A[i][k]);
                    j = i; /* j = row index of pivot */
                }
            }
        }
        pthread_barrier_wait(&barrier); /* all threads wait for pivot selection */

        if (k == myid)
        {
            if (max == 0)
            {
                printf("zero pivot: singular A matrix\n");
                exit(1);
            }
        }
        pthread_barrier_wait(&barrier);

        /* STEP 2 (thread k): Record the row swap in P[], swap rows of A and L */
        if (k == myid)
        {
            itemp = P[k]; P[k] = P[j]; P[j] = itemp;
        }
        pthread_barrier_wait(&barrier);

        if (k == myid)
        {
            printf("swap row %d and row %d of A on thread %d\n", k, j, myid);
            for (m = 0; m < n; m++)
            {
                temp = A[k][m]; A[k][m] = A[j][m]; A[j][m] = temp;
            }
            print('A', A);
        }
        pthread_barrier_wait(&barrier);

        /*
         * STEP 3: Swap already-computed L entries for columns 0..k-2
         * (the part of L we've already filled in previous steps).
         *
         * BUG FIX: original loop was `for (m = 0; m < k - 2; m++)`.
         * When k < 2 and k is int, k-2 is negative, so the loop body never runs
         * (safe but confusing). Added explicit guard for clarity.
         * The correct range is 0 .. k-1 (all L columns filled so far).
         */
        if (k >= 1)
        {
            for (m = 0; m < k; m++)
            {
                if (k == myid)
                {
                    temp = L[k][m]; L[k][m] = L[j][m]; L[j][m] = temp;
                }
                pthread_barrier_wait(&barrier);
            }
        }

        /* STEP 4 (thread k): Fill in U[k][k..n-1] and L[k+1..n-1][k] */
        if (k == myid)
        {
            U[k][k] = A[k][k];
            for (i = k + 1; i < n; i++)
            {
                L[i][k] = A[i][k] / U[k][k]; /* elimination multipliers */
                U[k][i] = A[k][i];
            }
        }
        pthread_barrier_wait(&barrier);

        /* STEP 5 (thread k): Row-reduce A using the multipliers just computed.
         * This zeroes out column k below the pivot, updating A for future steps. */
        if (k == myid)
        {
            printf("row reductions of A by thread %d on row %d\n", myid, k);
            for (i = k + 1; i < n; i++)
            {
                for (m = k + 1; m < n; m++)
                {
                    A[i][m] -= L[i][k] * U[k][m];
                }
            }
        }
        pthread_barrier_wait(&barrier);

        if (k == myid)
        {
            print('A', A);
            print('L', L);
            print('U', U);
            printP();
        }
        pthread_barrier_wait(&barrier);
    }

    return NULL; /* BUG FIX: pthread start routines must return void* */
}

int main(int argc, char *argv[])
{
    int i, j;

    n = N;

    printf("main: initialize matrix A[N][N], B[N], L, U and P\n");

    /* Initialize A: all 1s, with N on the anti-diagonal */
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
            A[i][j] = 1.0;
    for (i = 0; i < n; i++)
        A[i][N - 1 - i] = 1.0 * n;

    /* B[i] = n*(n+1)/2 + (n-i)*(n-1) — constructed so we know the solution */
    for (i = 0; i < n; i++)
        B[i] = (n) * (n + 1) / 2 + (n - i) * (n - 1);

    /* L starts as identity, U starts as zero */
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
        {
            U[i][j] = 0.0;
            L[i][j] = 0.0;
            if (i == j) L[i][j] = 1.0;
        }
    }

    /* P starts as [0, 1, 2, ..., n-1] (identity permutation) */
    for (i = 0; i < n; i++)
        P[i] = i;

    print('A', A);
    print('L', L);
    print('U', U);
    printV('B', B);
    printP();

    pthread_t threads[N];
    pthread_barrier_init(&barrier, NULL, N); /* barrier releases when all N threads arrive */

    printf("main: create N=%d working threads\n", N);
    for (i = 0; i < N; i++)
    {
        pthread_create(&threads[i], NULL, lu, (void *)(long)i);
    }

    printf("main: wait for all %d working threads to join\n", N);
    for (i = 0; i < N; i++)
    {
        pthread_join(threads[i], NULL);
    }

    /* All N threads have finished; P, L, U are fully computed.
     * Now solve A*X = B using the factored form P*L*U*X = B. */

    /* Step 1: apply permutation P to B → b */
    printV('B', B);
    for (i = 0; i < N; i++)
        b[i] = B[P[i]];
    printV('b', b);

    /* Step 2: Forward substitution — solve L*Y = b
     * L is lower triangular with 1s on diagonal, so Y[i] = b[i] - sum(L[i][j]*Y[j]) */
    for (i = 0; i < n; i++)
    {
        Y[i] = b[i];
        for (j = 0; j < i; j++)
            Y[i] -= L[i][j] * Y[j];
    }
    printV('Y', Y);

    /* Step 3: Backward substitution — solve U*X = Y
     * U is upper triangular, so we solve from bottom row up. */
    for (i = n - 1; i >= 0; i--)
    {
        double si = 0.0;
        for (j = i + 1; j < n; j++)
            si += U[i][j] * X[j];
        X[i] = (Y[i] - si) / U[i][i];
    }
    printV('X', X);

    pthread_barrier_destroy(&barrier);
    return 0;
}
