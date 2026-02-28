/** pivot.c: Gaussian Elimination with Partial Pivoting (multi-threaded) **/
/*
 * Gaussian Elimination reduces A to row echelon form in-place.
 * Unlike lu.c which produces separate L, U, P matrices, this combines
 * everything into an augmented matrix [A|B] and solves directly.
 *
 * The algorithm for each pivot row i (0..N-2):
 *   1. Partial pivoting: find row j >= i with largest |A[j][i]|, swap rows
 *   2. Row reduction: for each row j > i, subtract a multiple of row i
 *      (the "factor") to zero out column i
 *
 * Threading:
 *   Threads share the pivoting round-robin (thread i%nthreads handles step i).
 *   All threads participate in row reduction in parallel.
 *   Two barriers per iteration: one after pivoting, one after reduction.
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define N 8
int nthreads;
double A[N][N + 1]; /* augmented matrix [A|B]: N rows, N+1 columns (last col = B) */
pthread_barrier_t barrier;

/* BUG FIX: return type was int; this function returns no value */
void print_matrix(void)
{
    int i, j;
    printf("------------------------------------\n");
    for (i = 0; i < N; i++)
    {
        for (j = 0; j < N + 1; j++)
            printf("%6.2f  ", A[i][j]);
        printf("\n");
    }
}

/*
 * ge: thread worker for Gaussian Elimination.
 *
 * Each thread handles pivot rows where (i % nthreads == myid).
 * After pivoting, ALL threads cooperate on row reduction (also modulo nthreads).
 * Barriers enforce ordering: no thread starts reduction before pivoting is done.
 */
void *ge(void *arg)
{
    int i, j, k, prow;
    int myid = (int)(long)arg;
    double temp, factor;

    for (i = 0; i < N - 1; i++)
    {
        /* PIVOTING PHASE: only thread (i % nthreads) handles this step */
        if ((i % nthreads) == myid)
        {
            printf("partial pivoting by thread %d on row %d: ", myid, i);
            temp = 0.0;
            prow = i;

            /*
             * BUG FIX: original loop was `for (j = i; j <= N; j++)`.
             * A[N][i] is OUT OF BOUNDS — A is declared as A[N][N+1], so valid
             * row indices are 0..N-1. The loop must use j < N.
             */
            for (j = i; j < N; j++)
            {
                if (fabs(A[j][i]) > temp)
                {
                    temp = fabs(A[j][i]);
                    prow = j; /* track row with largest absolute value in column i */
                }
            }
            printf("pivot_row=%d  pivot=%6.2f\n", prow, A[prow][i]);

            /* Swap rows i and prow (swap all columns including B column) */
            if (prow != i)
            {
                for (j = i; j < N + 1; j++)
                {
                    temp = A[i][j];
                    A[i][j] = A[prow][j];
                    A[prow][j] = temp;
                }
            }
        }

        /* Barrier: wait for pivot/swap to complete before any thread reduces */
        pthread_barrier_wait(&barrier);

        /*
         * ROW REDUCTION PHASE: all threads participate, splitting rows by modulo.
         * For each row j below the pivot row i, subtract factor * row_i from row_j
         * to zero out column i. The factor = A[j][i] / A[i][i].
         *
         * Note: we only update columns i+1 through N (inclusive of B column).
         * Column i itself is set to 0 explicitly afterward.
         */
        for (j = i + 1; j < N; j++)
        {
            if ((j % nthreads) == myid)
            {
                printf("thread %d do row %d\n", myid, j);
                factor = A[j][i] / A[i][i];
                for (k = i + 1; k <= N; k++) /* k goes through N (the B column) */
                    A[j][k] -= A[i][k] * factor;
                A[j][i] = 0.0; /* explicitly zero (avoids floating point residual) */
            }
        }

        /* Barrier: wait for all row reductions before next pivot step */
        pthread_barrier_wait(&barrier);

        if (i == myid)
            print_matrix();
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int i, j;
    double sum;
    pthread_t threads[N];

    printf("enter number of threads (1-8): ");
    scanf("%d", &nthreads);

    /* Initialize [A|B]: A is all 1s with N on the anti-diagonal;
     * B (last column) is set so the system has a known integer solution. */
    printf("main: initialize matrix A[N][N+1] as [A|B]\n");
    for (i = 0; i < N; i++)
        for (j = 0; j < N; j++)
            A[i][j] = 1.0;
    for (i = 0; i < N; i++)
        A[i][N - i - 1] = 1.0 * N;
    for (i = 0; i < N; i++)
        A[i][N] = (N * (N + 1)) / 2 + (N - i) * (N - 1); /* last col = B */

    print_matrix();

    pthread_barrier_init(&barrier, NULL, nthreads);

    printf("main: create N=%d working threads\n", nthreads);
    for (i = 0; i < nthreads; i++)
        pthread_create(&threads[i], NULL, ge, (void *)(long)i);

    printf("main: wait for all %d working threads to join\n", nthreads);
    for (i = 0; i < nthreads; i++)
        pthread_join(threads[i], NULL);

    /*
     * Backward substitution: A is now in upper triangular form.
     * Solve from row N-1 upward. For each row i:
     *   X[i] = (B[i] - sum(A[i][j] * X[j], j > i)) / A[i][i]
     * The solution is stored back into A[i][N] for convenience.
     */
    printf("main: back substitution : ");
    for (i = N - 1; i >= 0; i--)
    {
        sum = 0.0;
        for (j = i + 1; j < N; j++)
            sum += A[i][j] * A[j][N];
        A[i][N] = (A[i][N] - sum) / A[i][i];
    }

    printf("The solution is :\n");
    for (i = 0; i < N; i++)
        printf("%6.2f  ", A[i][N]);
    printf("\n");

    pthread_barrier_destroy(&barrier);
    return 0;
}
