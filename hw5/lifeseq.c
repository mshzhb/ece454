/*****************************************************************************
 * life.c
 * The original sequential implementation resides here.
 * Do not modify this file, but you are encouraged to borrow from it
 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/**
 * Swapping the two boards only involves swapping pointers, not
 * copying values.
 */
#define SWAP_BOARDS( b1, b2 )  do { \
  char* temp = b1; \
  b1 = b2; \
  b2 = temp; \
} while(0)

#define BOARD( __board, __i, __j )  (__board[(__i) + LDA*(__j)])

void * process (void *ptr);

typedef struct Param {
    char * inboard;
    char * outboard;
    int ncols;
    int LDA;
    int start_row;
    int end_row;
    int nrows;
} Param;

char*
sequential_game_of_life (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    /* HINT: in the parallel decomposition, LDA may not be equal to
       nrows! */
    const int LDA = nrows;
    int curgen, i, j;

    /*
    for (curgen = 0; curgen < gens_max; curgen++)
    {

        for (i = 0; i < nrows; i++)
        {
            for (j = 0; j < ncols; j++)
            {
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);

                const char neighbor_count = 
                    BOARD (inboard, inorth, jwest) + 
                    BOARD (inboard, inorth, j) + 
                    BOARD (inboard, inorth, jeast) + 
                    BOARD (inboard, i, jwest) +
                    BOARD (inboard, i, jeast) + 
                    BOARD (inboard, isouth, jwest) +
                    BOARD (inboard, isouth, j) + 
                    BOARD (inboard, isouth, jeast);

                BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));

            }
        }
        SWAP_BOARDS( outboard, inboard );

    }
    */
    /* 
     * We return the output board, so that we know which one contains
     * the final result (because we've been swapping boards around).
     * Just be careful when you free() the two boards, so that you don't
     * free the same one twice!!! 
     */
	
    //first, set up the neighbour counts within the inboard and outboard    


    int num_threads = 1;
    pthread_t thread[num_threads];
    Param *ptr = malloc(num_threads*sizeof(Param));
    
    int start_row = 0;
    int chunk_row = nrows/num_threads;
    int end_row = chunk_row;
    
    
    for (i=0; i<num_threads; i++){
        
        ptr[i].start_row = start_row;
        ptr[i].end_row = end_row;
        start_row = end_row;
        end_row = end_row + chunk_row;

        ptr[i].nrows = nrows;
        ptr[i].ncols = ncols;
        ptr[i].LDA = nrows;
        printf ("%d to %d\n", ptr[i].start_row, ptr[i].end_row);
    }
    
    
    for (curgen = 0; curgen < gens_max; curgen++)
    {
        for (i=0; i<num_threads; i++){
            ptr[i].inboard = inboard;
            ptr[i].outboard = outboard;
            
            pthread_create(&thread[i], NULL, process, (void*) &ptr[i]);
        }

        for (i=0; i<num_threads; i++){
            pthread_join(thread[i],NULL);
        }

        SWAP_BOARDS( outboard, inboard );

    }
   
    return inboard;
}


void * process (void *ptr) {
    
    Param *p = (Param *) ptr;

    int start_row = p->start_row;
    int end_row = p->end_row;
    int ncols = p->ncols;
    int nrows = p->nrows;
    char * inboard = p-> inboard;
    char * outboard = p-> outboard;
    int i;
    int j;
    int LDA = p->LDA;
    
 
    /* HINT: you'll be parallelizing these loop(s) by doing a
       geometric decomposition of the output */
    for (i = start_row; i < end_row; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            const int inorth = mod (i-1, nrows);
            const int isouth = mod (i+1, nrows);
            const int jwest = mod (j-1, ncols);
            const int jeast = mod (j+1, ncols);

            const char neighbor_count = 
                BOARD (inboard, inorth, jwest) + 
                BOARD (inboard, inorth, j) + 
                BOARD (inboard, inorth, jeast) + 
                BOARD (inboard, i, jwest) +
                BOARD (inboard, i, jeast) + 
                BOARD (inboard, isouth, jwest) +
                BOARD (inboard, isouth, j) + 
                BOARD (inboard, isouth, jeast);

            BOARD(outboard, i, j) = alivep (neighbor_count, BOARD (inboard, i, j));

        }
    }

}


