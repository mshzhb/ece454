/*****************************************************************************
 life_neighbour_count.c


--- Cell Information ---
Each cell is represented  Byte of information
The Byte of information contains the following:
first 4 bits hold the neighbour_count
    neighbour_count is the number of neighbours that are alive 
5th bit is the cell's state (1 or 0)
6th bit is the clean bit
    syncronizes neighbour_count incr/decr across the two boards
7th bit is not used and is zero
8th bit is not used and is zero
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

#define IS_ALIVE(var) ((var) & (1<<(4))) 
#define IS_CLEAN(var) (!((var) & (1<<(5)))) 

#define SET_ALIVE(var)  (var |=  (1 << (4)))
#define SET_DEAD(var)  (var &= ~(1 << (4))) 
#define SET_CLEAN(var) (var &= ~(1<<5))

#define SET_DIRTY(var)  (var |=  (1 << (5)))


#define INCR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] ++ )
#define DECR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] -- )
#define COUNT_OF_BOARD(__board, __i, __j )  (__board[(__i) + nrows*(__j)] & (char)0x0f  )
#define TOP_4_BITS(__board, __i, __j )  (__board[(__i) + nrows*(__j)] & (char)0x30  )

void incr_neighbours2(char * board, int index, int nrows, int ncols);
void * process (void *ptr);
void  process_single_row (int i, int ncols, int nrows, char * inboard, char * outboard, int LDA);


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
nc_game_of_life (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{

    const int LDA = nrows;
    int curgen, i, j;


    int num_threads = 4;
    pthread_t thread[num_threads];
    Param *ptr = malloc(num_threads*sizeof(Param));
    
    int start_row = 0;
    int chunk_row = nrows/num_threads;
    int end_row = chunk_row;
    
    //set up param for threads
    for (i=0; i<num_threads; i++){
        
        ptr[i].start_row = start_row+1;
        ptr[i].end_row = end_row-1;
        //ptr[i].start_row = start_row;
        //ptr[i].end_row = end_row;
        
        start_row = end_row;
        end_row = end_row + chunk_row;

        ptr[i].nrows = nrows;
        ptr[i].ncols = ncols;
        ptr[i].LDA = nrows;
        printf ("%d to %d\n", ptr[i].start_row, ptr[i].end_row);
    }
    

    for (curgen = 0; curgen < gens_max; curgen++)
    {
        //we have to process the first and last row of each chunk without parallelization
        //to avoid threads writing to the same neighbouring threads
        
        
        for (i=0; i<num_threads; i++){
            process_single_row( ptr[i].start_row-1 , ncols, nrows, inboard, outboard, LDA);
            process_single_row( ptr[i].end_row , ncols, nrows, inboard, outboard, LDA);
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

    //fix output, shift all to the right >> 4
    for (i = 0; i < nrows; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            SET_CLEAN(BOARD(inboard,i,j));
            BOARD(inboard,i,j) = BOARD(inboard,i,j) >> 4;
        }
    }
    free(ptr);

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
    
    //reading inboard with column-major
    for (j = 0; j < ncols; j++)
    {
        for (i = start_row; i < end_row; i++)
        {
            //set clean bit to zero for inboard to invalidate inboard's neighbour count for next iteration
            SET_CLEAN(BOARD(inboard,i,j));


            char cell = BOARD(inboard,i,j);
            int alive = cell >> 4;
            
            //if cell is dead and has 3 neighbouring cells
            if (!alive) {
                if (cell == (char)0x03 ) {
                    
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);
            
                    //we have to do these if statements first to ensure neighbour_count is accurate in outboard
                    //if cell is clean, we will set inboard's neighbour_count to outboard's neighbour_count
                    if (IS_CLEAN(BOARD (outboard, inorth, jwest)))
                        BOARD(outboard,inorth, jwest) = TOP_4_BITS(outboard, inorth, jwest) | COUNT_OF_BOARD(inboard, inorth, jwest);

                    if (IS_CLEAN(BOARD (outboard, inorth, j)))
                        BOARD(outboard,inorth, j) = TOP_4_BITS(outboard, inorth, j) | COUNT_OF_BOARD(inboard, inorth, j);

                    if (IS_CLEAN(BOARD (outboard, inorth, jeast)))
                        BOARD(outboard,inorth, jeast) = TOP_4_BITS(outboard, inorth, jeast) | COUNT_OF_BOARD(inboard, inorth, jeast);

                    if (IS_CLEAN(BOARD (outboard, i, jwest)))
                        BOARD(outboard,i, jwest) = TOP_4_BITS(outboard, i, jwest) | COUNT_OF_BOARD(inboard, i, jwest);

                    if (IS_CLEAN(BOARD (outboard, i, jeast)))
                        BOARD(outboard,i, jeast) = TOP_4_BITS(outboard, i, jeast) | COUNT_OF_BOARD(inboard, i, jeast);

                    if (IS_CLEAN(BOARD (outboard, isouth, jwest)))
                        BOARD(outboard,isouth, jwest) = TOP_4_BITS(outboard, isouth, jwest) | COUNT_OF_BOARD(inboard, isouth, jwest);

                    if (IS_CLEAN(BOARD (outboard, isouth, j)))
                        BOARD(outboard, isouth, j) = TOP_4_BITS(outboard, isouth, j) | COUNT_OF_BOARD(inboard, isouth, j);

                    if (IS_CLEAN(BOARD (outboard, isouth, jeast)))
                        BOARD(outboard,isouth, jeast) = TOP_4_BITS(outboard, isouth, jeast) | COUNT_OF_BOARD(inboard, isouth, jeast);
                    
                    //set the cell in both boards to the correct state
                    SET_ALIVE(BOARD(outboard,i,j));
                    SET_ALIVE(BOARD(inboard,i,j));
                    
                    //increment the neighbour_count of outboard
                    //once process is finished, neighbour_count of outboard
                    //will represent current accurate neighbour_count
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
            } else if (alive) { //if cell is alive and needs to die
                if (cell <= (char)0x11 || cell >= (char)0x14) {
             
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);
                
                    
                    if (IS_CLEAN(BOARD (outboard, inorth, jwest)))
                        BOARD(outboard,inorth, jwest) = TOP_4_BITS(outboard, inorth, jwest) | COUNT_OF_BOARD(inboard, inorth, jwest);

                    if (IS_CLEAN(BOARD (outboard, inorth, j)))
                        BOARD(outboard,inorth, j) = TOP_4_BITS(outboard, inorth, j) | COUNT_OF_BOARD(inboard, inorth, j);

                    if (IS_CLEAN(BOARD (outboard, inorth, jeast)))
                        BOARD(outboard,inorth, jeast) = TOP_4_BITS(outboard, inorth, jeast) | COUNT_OF_BOARD(inboard, inorth, jeast);

                    if (IS_CLEAN(BOARD (outboard, i, jwest)))
                        BOARD(outboard,i, jwest) = TOP_4_BITS(outboard, i, jwest) | COUNT_OF_BOARD(inboard, i, jwest);

                    if (IS_CLEAN(BOARD (outboard, i, jeast)))
                        BOARD(outboard,i, jeast) = TOP_4_BITS(outboard, i, jeast) | COUNT_OF_BOARD(inboard, i, jeast);

                    if (IS_CLEAN(BOARD (outboard, isouth, jwest)))
                        BOARD(outboard,isouth, jwest) = TOP_4_BITS(outboard, isouth, jwest) | COUNT_OF_BOARD(inboard, isouth, jwest);

                    if (IS_CLEAN(BOARD (outboard, isouth, j)))
                        BOARD(outboard, isouth, j) = TOP_4_BITS(outboard, isouth, j) | COUNT_OF_BOARD(inboard, isouth, j);

                    if (IS_CLEAN(BOARD (outboard, isouth, jeast)))
                        BOARD(outboard,isouth, jeast) = TOP_4_BITS(outboard, isouth, jeast) | COUNT_OF_BOARD(inboard, isouth, jeast);

                    SET_DEAD(BOARD(outboard,i,j));
                    SET_DEAD(BOARD(inboard,i,j));

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

            //if current cell does not need to be updated
            //we simply update outboard's with inboard's information
            if (IS_CLEAN(BOARD (outboard, i, j))) {
                BOARD(outboard,i, j) = BOARD(inboard, i, j);
                SET_DIRTY (BOARD(outboard, i, j));
            }

        }
    }

}




void  process_single_row (int i, int ncols, int nrows, char * inboard, char * outboard, int LDA) {
    
    int j;
    for (j = 0; j < ncols; j++)
    {
        //set clean bit to zero for inboard to invalidate inboard's neighbour count for next iteration
        SET_CLEAN(BOARD(inboard,i,j));


        char cell = BOARD(inboard,i,j);
        int alive = cell >> 4;
        
        //if cell is dead and has 3 neighbouring cells
        if (!alive) {
            if (cell == (char)0x03 ) {
                
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
        
                //we have to do these if statements first to ensure neighbour_count is accurate in outboard
                //if cell is clean, we will set inboard's neighbour_count to outboard's neighbour_count
                if (IS_CLEAN(BOARD (outboard, inorth, jwest)))
                    BOARD(outboard,inorth, jwest) = TOP_4_BITS(outboard, inorth, jwest) | COUNT_OF_BOARD(inboard, inorth, jwest);

                if (IS_CLEAN(BOARD (outboard, inorth, j)))
                    BOARD(outboard,inorth, j) = TOP_4_BITS(outboard, inorth, j) | COUNT_OF_BOARD(inboard, inorth, j);

                if (IS_CLEAN(BOARD (outboard, inorth, jeast)))
                    BOARD(outboard,inorth, jeast) = TOP_4_BITS(outboard, inorth, jeast) | COUNT_OF_BOARD(inboard, inorth, jeast);

                if (IS_CLEAN(BOARD (outboard, i, jwest)))
                    BOARD(outboard,i, jwest) = TOP_4_BITS(outboard, i, jwest) | COUNT_OF_BOARD(inboard, i, jwest);

                if (IS_CLEAN(BOARD (outboard, i, jeast)))
                    BOARD(outboard,i, jeast) = TOP_4_BITS(outboard, i, jeast) | COUNT_OF_BOARD(inboard, i, jeast);

                if (IS_CLEAN(BOARD (outboard, isouth, jwest)))
                    BOARD(outboard,isouth, jwest) = TOP_4_BITS(outboard, isouth, jwest) | COUNT_OF_BOARD(inboard, isouth, jwest);

                if (IS_CLEAN(BOARD (outboard, isouth, j)))
                    BOARD(outboard, isouth, j) = TOP_4_BITS(outboard, isouth, j) | COUNT_OF_BOARD(inboard, isouth, j);

                if (IS_CLEAN(BOARD (outboard, isouth, jeast)))
                    BOARD(outboard,isouth, jeast) = TOP_4_BITS(outboard, isouth, jeast) | COUNT_OF_BOARD(inboard, isouth, jeast);
                
                //set the cell in both boards to the correct state
                SET_ALIVE(BOARD(outboard,i,j));
                SET_ALIVE(BOARD(inboard,i,j));
                
                //increment the neighbour_count of outboard
                //once process is finished, neighbour_count of outboard
                //will represent current accurate neighbour_count
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
        } else if (alive) { //if cell is alive and needs to die
            if (cell <= (char)0x11 || cell >= (char)0x14) {
         
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
            
                
                if (IS_CLEAN(BOARD (outboard, inorth, jwest)))
                    BOARD(outboard,inorth, jwest) = TOP_4_BITS(outboard, inorth, jwest) | COUNT_OF_BOARD(inboard, inorth, jwest);

                if (IS_CLEAN(BOARD (outboard, inorth, j)))
                    BOARD(outboard,inorth, j) = TOP_4_BITS(outboard, inorth, j) | COUNT_OF_BOARD(inboard, inorth, j);

                if (IS_CLEAN(BOARD (outboard, inorth, jeast)))
                    BOARD(outboard,inorth, jeast) = TOP_4_BITS(outboard, inorth, jeast) | COUNT_OF_BOARD(inboard, inorth, jeast);

                if (IS_CLEAN(BOARD (outboard, i, jwest)))
                    BOARD(outboard,i, jwest) = TOP_4_BITS(outboard, i, jwest) | COUNT_OF_BOARD(inboard, i, jwest);

                if (IS_CLEAN(BOARD (outboard, i, jeast)))
                    BOARD(outboard,i, jeast) = TOP_4_BITS(outboard, i, jeast) | COUNT_OF_BOARD(inboard, i, jeast);

                if (IS_CLEAN(BOARD (outboard, isouth, jwest)))
                    BOARD(outboard,isouth, jwest) = TOP_4_BITS(outboard, isouth, jwest) | COUNT_OF_BOARD(inboard, isouth, jwest);

                if (IS_CLEAN(BOARD (outboard, isouth, j)))
                    BOARD(outboard, isouth, j) = TOP_4_BITS(outboard, isouth, j) | COUNT_OF_BOARD(inboard, isouth, j);

                if (IS_CLEAN(BOARD (outboard, isouth, jeast)))
                    BOARD(outboard,isouth, jeast) = TOP_4_BITS(outboard, isouth, jeast) | COUNT_OF_BOARD(inboard, isouth, jeast);

                SET_DEAD(BOARD(outboard,i,j));
                SET_DEAD(BOARD(inboard,i,j));

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

        //if current cell does not need to be updated
        //we simply update outboard's with inboard's information
        if (IS_CLEAN(BOARD (outboard, i, j))) {
            BOARD(outboard,i, j) = BOARD(inboard, i, j);
            SET_DIRTY (BOARD(outboard, i, j));
        }

    }


}


