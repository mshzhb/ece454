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


#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define IS_ALIVE(var) ((var) & (1<<(4))) //check if the 5th bit is set --> alive
#define IS_CLEAN(var) (!((var) & (1<<(5)))) //if 6th bit is zero --> is clean

#define SET_ALIVE(var)  (var |=  (1 << (4)))
#define SET_DEAD(var)  (var &= ~(1 << (4))) 
#define SET_CLEAN(var) (var &= ~(1<<5))

#define SET_DIRTY(var)  (var |=  (1 << (5)))


#define TOGGLE_STATE(var) (var ^= 1 << 4)


#define INCR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] ++ )
#define DECR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] -- )




void incr_neighbours2(char * board, int index, int nrows, int ncols);
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

    printf("\n---BEGIN\n");
    printf("\n---INBOARD\n");

    for (i = 0; i < nrows; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            char cell = BOARD(inboard,i,j);
            printf("\nat %x ",cell);
        }
    }
    printf("\n---BEGIN\n");
    printf("\n---OUTBOARD\n");

    for (i = 0; i < nrows; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            char cell = BOARD(outboard,i,j);
            printf("\nat %x ",cell);
        }
    }

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
         printf("\n---CURGEN\n");
        printf("\n---INBOARD\n");

        for (i = 0; i < nrows; i++)
        {
            for (j = 0; j < ncols; j++)
            {
                char cell = BOARD(inboard,i,j);
                printf("\nat %x ",cell);
            }
        }
        printf("\n---CURGEN\n");
        printf("\n---OUTBOARD\n");

        for (i = 0; i < nrows; i++)
        {
            for (j = 0; j < ncols; j++)
            {
                char cell = BOARD(outboard,i,j);
                printf("\nat %x ",cell);
            }
        }
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

    printf("\n---COMPLETE\n");
    for (i = 0; i < nrows; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            char cell = BOARD(inboard,i,j);
            printf("\nat %x ",cell);
        }
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
    
    printf("\n----PROCESS\n");
    /* HINT: you'll be parallelizing these loop(s) by doing a
       geometric decomposition of the output */
    for (i = start_row; i < end_row; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            /*
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
            */
            //printf("%x\n",inboard[i+end_row*j]);

            //reset clean bit for inboard for next iteration
            SET_CLEAN(BOARD(inboard,i,j));

            char cell = BOARD(inboard,i,j);
            int alive = cell >> 4;
            printf("\nat %x with state %d ",cell, alive);
            if (!alive) {
                if (cell == 0x03) {
                    printf("YEA! going to be alive!");
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);

                    if (IS_CLEAN(BOARD (outboard, inorth, jwest)))
                        BOARD (outboard, inorth, jwest) = BOARD (inboard, inorth, jwest);

                    if (IS_CLEAN(BOARD (outboard, inorth, j)))
                        BOARD (outboard, inorth, j) = BOARD (inboard, inorth, j);

                    if (IS_CLEAN(BOARD (outboard, inorth, jeast)))
                        BOARD (outboard, inorth, jeast) = BOARD (inboard, inorth, jeast);

                    if (IS_CLEAN(BOARD (outboard, i, jwest)))
                        BOARD (outboard, i, jwest) = BOARD (inboard, i, jwest);

                    if (IS_CLEAN(BOARD (outboard, i, jeast)))
                        BOARD (outboard, i, jeast) = BOARD (inboard, i, jeast);

                    if (IS_CLEAN(BOARD (outboard, isouth, jwest)))
                        BOARD (outboard, isouth, jwest) = BOARD (inboard, isouth, jwest);

                    if (IS_CLEAN(BOARD (outboard, isouth, j)))
                        BOARD (outboard, isouth, j) = BOARD (inboard, isouth, j);

                    if (IS_CLEAN(BOARD (outboard, isouth, jeast)))
                        BOARD (outboard, isouth, jeast) = BOARD (inboard, isouth, jeast);

                    SET_ALIVE(BOARD(outboard,i,j));

                    INCR_AT (outboard, inorth, jwest);
                    INCR_AT (outboard, inorth, j);
                    INCR_AT (outboard, inorth, jeast);
                    INCR_AT (outboard, i, jwest);
                    INCR_AT (outboard, i, jeast);
                    INCR_AT (outboard, isouth, jwest);
                    INCR_AT (outboard, isouth, j);
                    INCR_AT (outboard, isouth, jeast);
                    
                    SET_DIRTY (BOARD(outboard, inorth, jwest));
                    SET_DIRTY (BOARD(outboard, inorth, j));
                    SET_DIRTY (BOARD(outboard, inorth, jeast));
                    SET_DIRTY (BOARD(outboard, i, jwest));
                    SET_DIRTY (BOARD(outboard, i, jeast));
                    SET_DIRTY (BOARD(outboard, isouth, jwest));
                    SET_DIRTY (BOARD(outboard, isouth, j));
                    SET_DIRTY (BOARD(outboard, isouth, jeast));

                }
            } else if (alive) {
                if (cell <= 0x11 || cell >=0x14) {
                    printf("FOUND A LIFE GONNA DIE");
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);

                    if (IS_CLEAN(BOARD (outboard, inorth, jwest)))
                        BOARD (outboard, inorth, jwest) = BOARD (inboard, inorth, jwest);

                    if (IS_CLEAN(BOARD (outboard, inorth, j)))
                        BOARD (outboard, inorth, j) = BOARD (inboard, inorth, j);

                    if (IS_CLEAN(BOARD (outboard, inorth, jeast)))
                        BOARD (outboard, inorth, jeast) = BOARD (inboard, inorth, jeast);

                    if (IS_CLEAN(BOARD (outboard, i, jwest)))
                        BOARD (outboard, i, jwest) = BOARD (inboard, i, jwest);

                    if (IS_CLEAN(BOARD (outboard, i, jeast)))
                        BOARD (outboard, i, jeast) = BOARD (inboard, i, jeast);

                    if (IS_CLEAN(BOARD (outboard, isouth, jwest)))
                        BOARD (outboard, isouth, jwest) = BOARD (inboard, isouth, jwest);

                    if (IS_CLEAN(BOARD (outboard, isouth, j)))
                        BOARD (outboard, isouth, j) = BOARD (inboard, isouth, j);

                    if (IS_CLEAN(BOARD (outboard, isouth, jeast)))
                        BOARD (outboard, isouth, jeast) = BOARD (inboard, isouth, jeast);

                    SET_DEAD(BOARD(outboard,i,j));

                    DECR_AT (outboard, inorth, jwest);
                    DECR_AT (outboard, inorth, j);
                    DECR_AT (outboard, inorth, jeast);
                    DECR_AT (outboard, i, jwest);
                    DECR_AT (outboard, i, jeast);
                    DECR_AT (outboard, isouth, jwest);
                    DECR_AT (outboard, isouth, j);
                    DECR_AT (outboard, isouth, jeast);
                    
                    SET_DIRTY (BOARD(outboard, inorth, jwest));
                    SET_DIRTY (BOARD(outboard, inorth, j));
                    SET_DIRTY (BOARD(outboard, inorth, jeast));
                    SET_DIRTY (BOARD(outboard, i, jwest));
                    SET_DIRTY (BOARD(outboard, i, jeast));
                    SET_DIRTY (BOARD(outboard, isouth, jwest));
                    SET_DIRTY (BOARD(outboard, isouth, j));
                    SET_DIRTY (BOARD(outboard, isouth, jeast));
                    

                }
            }



        }
    }

}


