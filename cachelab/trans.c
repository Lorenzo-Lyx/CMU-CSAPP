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


#include <time.h>
#include <stdlib.h>

int is_transpose(int M, int N, int A[N][M], int B[M][N]);

void printMatrix(int N, int M, int matrix[N][M]);

/* 
 * transpose_submit - This is the solution transpose function that you
 *     will be graded on for Part B of the assignment. Do not change
 *     the description string "Transpose submission", as the driver
 *     searches for that string to identify the transpose function to
 *     be graded. 
 */
char transpose_submit_desc[] = "Transpose submission";
void transpose_submit(int M, int N, int A[N][M], int B[M][N])
{
    int r, c; int i, j; int x1, x2, x3, x4, x5, x6, x7, x8;
    // printMatrix(N, M, A);
    if(M == 32) {
        for(r = 0; r < N; r+=8){
            for(c = 0; c < M; c+=8){
                // if it is on the diag
                if (r == c) {
                    for(i = r; i < r+8; ++i) {
                        for (j = c; j < c+8; ++j) {
                            B[j][i] = A[(16+i)%32][(16+j)%32];
                        }
                    }
                }
                else {
                    for(i = r; i < r+8; ++i){
                        for(j = c; j < c+8; ++j){
                            B[j][i] = A[i][j];
                        }
                    }
                }
                
            }
        }
        // process A's diag, switch (0,0)and(2,2), (1,1)and(3,3)
        for(x1 = 0; x1 < 16; x1+=8) {
            for(r = x1, i = x1+16; r < x1+8; ++r, ++i) {
                for (c = x1, j = x1+16; c < x1+8; ++c, ++j) {
                    x8 = B[r][c];
                    B[r][c] = B[i][j];
                    B[i][j] = x8;                
                }
            }
        }
        // printMatrix(M, N, B);
    } else if (M == 64) {
        for(r = 0; r < 64; r += 8) {
            for(c = 0; c < 64; c += 8) {
                for(i = r, j = c; i < r+4; ++i, ++j) {

                    x1 = A[i][c]; x2 = A[i][c+1]; x3 = A[i][c+2]; x4 = A[i][c+3];
                    x5 = A[r][j+4]; x6 = A[r+1][j+4]; x7 = A[r+2][j+4]; x8 = A[r+3][j+4];
                    
                    B[c][i] = x1; B[c+1][i] = x2; B[c+2][i] = x3; B[c+3][i] = x4;
                    B[j][r+4]=x5; B[j][r+5]=x6; B[j][r+6]=x7; B[j][r+7]=x8;
                }
                // i = r+4; j = c+4
                for(j = c; j < c+4; ++j) {
                    x1 = A[r+4][j]; x2 = A[r+5][j]; x3 = A[r+6][j]; x4 = A[r+7][j];
                    
                    x5 = B[j][r+4]; x6 = B[j][r+5]; x7 = B[j][r+6]; x8 = B[j][r+7];
                    
                    B[j][r+4] = x1; B[j][r+5] = x2; B[j][r+6] = x3; B[j][r+7] = x4;

                    B[j+4][r] = x5; B[j+4][r+1] = x6; B[j+4][r+2] = x7; B[j+4][r+3] = x8;

                }
                for(i = r+4; i < r+8; ++i) {
                    x1 = A[i][c+4]; x2 = A[i][c+5]; x3 = A[i][c+6]; x4 = A[i][c+7];
                    B[c+4][i] = x1; B[c+5][i] = x2; B[c+6][i] = x3; B[c+7][i] = x4;
                }
                
            }
        }
            
    } else {
        for(r = 0; r < N/8 * 8; r+=8) {
            for (c = 0; c < M/ 8 * 8; c+=8) {
                for(i = r; i < r + 8; ++i) {
                    x1 = A[i][c]; x2 = A[i][c+1]; x3 = A[i][c+2]; x4 = A[i][c+3];
                    x5 = A[i][c+4]; x6 = A[i][c+5]; x7 = A[i][c+6]; x8 = A[i][c+7];
                    B[c][i] = x1; B[c+1][i] = x2; B[c+2][i] = x3; B[c+3][i] = x4;
                    B[c+4][i]=x5; B[c+5][i] = x6; B[c+6][i] = x7; B[c+7][i] = x8;
                }
            }
        }
        for(r = 0; r < N; ++r) {
            for(c = M/8 * 8; c < M; ++c) {
                B[c][r] = A[r][c];
            }
        }
        for (r = N/8 * 8; r < N; ++r) {
            for(c = 0; c < M/8 * 8; ++c) {
                B[c][r] = A[r][c];
            }
        }
    }
    
    return;
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


char trans_block8x8_desc[] = "32x32 0r 64x64 blocking version.";
void trans_block8x8_func(int M, int N, int A[N][M], int B[M][N]) {
    int bsize = M == 32 ? 8 : 4;
    for(int r = 0; r < N; r+=bsize){
        for(int c = 0; c < M; c+=bsize){
            int rLimit = r+bsize;
            for(int i = r; i < rLimit; ++i){
                int cLimit = c+bsize;
                for(int j = c; j < cLimit; ++j){
                    B[j][i] = A[i][j];
                }
            }
        }
    }
    return;
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

    registerTransFunction(trans_block8x8_func, trans_block8x8_desc);

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


// int main() {
//     int N = 8; int M = 8;
//     int A[N][M]; int B[M][N];
//     srand(time(NULL));
//     for (int i = 0; i < N; i++){
//         for (int j = 0; j < M; j++){
//             // A[i][j] = i+j;  /* The matrix created this way is symmetric */
//             A[i][j] = rand()%100;
//             B[j][i] = rand()%100;
//         }
//     }
//     transpose_submit(M, N, A, B);

//     for(int i = 0; i < N; ++i) {
//         for (int j = 0; j < M; ++j) {
//             if (A[i][j] != B[j][i]) {
//                 printf("A: (%d, %d)\n", i, j);
//             }
//         }
//     }

// }


void printMatrix(int N, int M, int matrix[N][M]) {
    printf("the matrix: \n");
    for(int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            printf("%4d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("print matrix over...\n");

    return;
}

