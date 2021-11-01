/* 
 * trans.c - Matrix transpose B = A^T
 *
 * Each transpose function must have a prototype of the form:
 * void trans(int M, int N, int A[N][M], int B[M][N]);
 *
 * A transpose function is evaluated by counting the number of misses
 * on a 1KB direct mapped cache with a block size of 32 bytes.
 */ 
#include <stdio.h>
#include "cachelab.h"

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */

void transpose_for_32(int M, int N, int A[N][M], int B[M][N]) {
    int block_size = 8;
    int ii, jj;
    int i, j;
    for (ii = 0; ii < N / block_size; ii ++) {
        for (jj = 0; jj < M / block_size; jj ++) {
            for (i = 0; i < block_size; i ++) {
                for (j = 0; j < block_size; j ++) {
                    if (i == j) continue;  //dialog element place last
                    B[jj * block_size + j][ii * block_size + i] = A[ii * block_size + i][jj * block_size + j];
                }
                B[jj * block_size + i][ii * block_size + i] = A[ii * block_size + i][jj * block_size + i];
            }
        }
    }
}

void transpose_for_64(int M, int N, int A[N][M], int B[M][N]) {
    int block_size = 4;
    int ii, jj;
    int i, j;
    for (ii = 0; ii < N / block_size; ii ++) {
        for (jj = 0; jj < M / block_size; jj ++) {
            for (i = 0; i < block_size; i ++) {
                for (j = 0; j < block_size; j ++) {
                    if (i == j) continue;  //dialog element place last
                    B[jj * block_size + j][ii * block_size + i] = A[ii * block_size + i][jj * block_size + j];
                }
                B[jj * block_size + i][ii * block_size + i] = A[ii * block_size + i][jj * block_size + i];
            }
        }
    }
}

void transpose_for_61(int M, int N, int A[N][M], int B[M][N]) {
    int block_size = 16;
    int ii, jj;
    int i, j;
    for (ii = 0; ii < N ; ii += block_size) {
        for (jj = 0; jj < M ; jj += block_size) {
            for (i = 0; i < block_size && i + ii < N; i ++) {
                for (j = 0; j < block_size && j + jj < M; j ++) {
                    B[jj + j][ii + i] = A[ii + i][jj + j];
                }
            }
        }
    }
}

char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    if (N == 32) transpose_for_32(M, N, A, B);
    else if (N == 64) transpose_for_64(M, N, A, B);
    else transpose_for_61(M, N, A, B);

}




char transpose_block_4[] = "split the matrix to 4 * 4";
void f_transpose_block_4(int M, int N, int A[N][M], int B[M][N])
{
    int ii, jj;
    int i, j;
    for (ii = 0; ii < N / 4; ii ++) {
        for (jj = 0; jj < M / 4; jj ++) {
            for (i = 0; i < 4; i ++) {
                for (j = 0; j < 4; j ++) {
                    B[jj * 4 + j][ii * 4 + i] = A[ii * 4 + i][jj * 4 + j];
                }
            }
        }
    }
}


/* 
 * You can define additional transpose functions below. We've defined
 * a simple one below to help you get started. 
 */ 

/* 
 * trans - A simple baseline transpose function, not optimized for the cache.
 */
char trans_desc[] = "Simple row-wise scan transpose";
void trans(int M, int N, int A[N][M], int B[M][N])
{
    int i, j, tmp;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            tmp = A[i][j];
            B[j][i] = tmp;
        }
    }    

}

/*
 * registerFunctions - This function registers your transpose
 *     functions with the driver.  At runtime, the driver will
 *     evaluate each of the registered functions and summarize their
 *     performance. This is a handy way to experiment with different
 *     transpose strategies.
 */
void registerFunctions()
{
    /* Register your solution function */
    registerTransFunction(transpose_submit, transpose_submit_desc); 

    /* Register any additional transpose functions */
    registerTransFunction(trans, trans_desc); 

    registerTransFunction(f_transpose_block_4, transpose_block_4);

}

/* 
 * is_transpose - This helper function checks if B is the transpose of
 *     A. You can check the correctness of your transpose by calling
 *     it before returning from the transpose function.
 */
int is_transpose(int M, int N, int A[N][M], int B[M][N])
{
    int i, j;

    for (i = 0; i < N; i++) {
        for (j = 0; j < M; ++j) {
            if (A[i][j] != B[j][i]) {
                return 0;
            }
        }
    }
    return 1;
}

