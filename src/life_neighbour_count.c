/*****************************************************************************
 life_neighbour_count.c

Michael Law - 997376343
Jonathan Ng - 997836141


The following describes the main optimization methods used in nc_game_of_life()
Extra information is stored within the cell to make use of 
as much memory as possible and parallelization allows the board to 
be processed much faster at each iteration.

---- Neighbour-Count Cell Representation ----
Each cell state is contained within a byte of information
0th, 1st, 2nd, 3rd bit holds the neighbour_count
    neighbour_count is the number of neighbours that are alive 
4th bit is the cell's state (1 or 0)
5th bit is not used and is zero
6th bit is not used and is zero
7th bit is not used and is zero

---- Parallelization with pthreads ----
Each thread takes processes a rectangular chunk of the board
and waits for every other thread to finish before moving onto
the next iteration of curgen


 ****************************************************************************/
#include "life.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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

//check if cell is alive/dead
#define IS_ALIVE(var) ((var) & (1<<(4))) 
#define IS_CLEAN(var) (!((var) & (1<<(5)))) 

//set cell states
#define SET_ALIVE(var)  (var |=  (1 << (4)))
#define SET_DEAD(var)  (var &= ~(1 << (4))) 
#define SET_CLEAN(var) (var &= ~(1<<5))
#define SET_DIRTY(var)  (var |=  (1 << (5)))

//increase or decrease neighbour counts
#define INCR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] ++ )
#define DECR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] -- )

//return the neighbour_count of the cell
#define COUNT_OF_BOARD(__board, __i, __j )  (__board[(__i) + nrows*(__j)] & (char)0x0f  )

//return the clean and state bits of the cell
#define TOP_4_BITS(__board, __i, __j )  (__board[(__i) + nrows*(__j)] & (char)0x30  )


void * process (void *ptr);
void  process_single_row (int i, int ncols, int nrows, char * inboard, char * outboard, int LDA);

/********************************
Param
This struct is used to pass information to threads
********************************/
typedef struct Param {
    char * inboard;
    char * outboard;
    int ncols;
    int LDA;
    int start_row;
    int end_row;
    int nrows;
} Param;

/********************************
nc_game_of_life
main function that drives game of life
using neighbour_count implementation
********************************/
char*
nc_game_of_life (char* outboard, 
        char* inboard,
        const int nrows,
        const int ncols,
        const int gens_max)
{
    //exiting gracefully if size of board exceeds 10000
    if (nrows > 10000) return inboard;

    const int LDA = nrows;
    int curgen, i, j;

    //parallelization setup
    int num_threads = 4;
    pthread_t thread[num_threads];
    Param *ptr = malloc(num_threads*sizeof(Param));
    
    int start_row = 0;
    int chunk_row = nrows/num_threads;
    int end_row = chunk_row;

    //set up inputs for threads
    for (i=0; i<num_threads; i++){

        ptr[i].start_row = start_row+1;
        ptr[i].end_row = end_row-1;
 
        start_row = end_row;
        end_row = end_row + chunk_row;
        
        ptr[i].nrows = nrows;
        ptr[i].ncols = ncols;
        ptr[i].LDA = nrows;
    }
    
    //begin generations of game of life
    /*
        First, we have to process the first and last row of each chunk 
        without parallelization to avoid threads writing to the same neighbouring cells.
        Then, we can parallelize and let the main thread wait for the 4 threads before
        the next iteration of Game of Life
    */
    for (curgen = 0; curgen < gens_max; curgen++)
    {
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
        
        //we swap the boards so inboard then contains the correct state (which was outboard)
        SWAP_BOARDS( outboard, inboard ); 
        //copy contents of inboard to outboard before next iteration
        memmove (outboard, inboard, nrows * ncols * sizeof (char));
	
    }

    /*
        We fix the output by shifting the cell state to the first bit
    */
    for (i = 0; i < nrows; i++)
    {
        for (j = 0; j < ncols; j++)
        {
            BOARD(inboard,i,j) = BOARD(inboard,i,j) >> 4;
        }
    }

    free(ptr);

    return inboard;

}

/********************************
process
threads will run process() to update the cell's state
the cell's state is updated according to the neighbour_count
if a cell state needs to be changed, we will update the neighbouring cell's
neighbour_count to reflect the new neighbour count in outboard
********************************/

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
    
    for (i = start_row; i < end_row; i++)
    {
       

        for (j = 0; j < ncols; j++)
        {
            char cell = BOARD(inboard,i,j);
            int alive = cell >> 4;
            
            //if cell is dead and has 3 neighbouring cells
            if (!alive) {
                if (cell == (char)0x03 ) {
                    
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);
                    
                    //set the cell in outboards to the correct state
                    SET_ALIVE(BOARD(outboard,i,j));
                    
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
                    
                }
            } else if (alive) { //if cell is alive and needs to die
                if (cell <= (char)0x11 || cell >= (char)0x14) {
             
                    const int inorth = mod (i-1, nrows);
                    const int isouth = mod (i+1, nrows);
                    const int jwest = mod (j-1, ncols);
                    const int jeast = mod (j+1, ncols);
                
                    
                    SET_DEAD(BOARD(outboard,i,j));

                    DECR_AT (outboard, inorth, jwest);
                    DECR_AT (outboard, inorth, j);
                    DECR_AT (outboard, inorth, jeast);
                    DECR_AT (outboard, i, jwest);
                    DECR_AT (outboard, i, jeast);
                    DECR_AT (outboard, isouth, jwest);
                    DECR_AT (outboard, isouth, j);
                    DECR_AT (outboard, isouth, jeast);
                   
                }
            }
        }
       
    }

}


/********************************
process_single_row
process the given row, i, from inboard, and stores the results in outboard.
this function is used to avoid a race case condition 
where the threads were going to access the same neighbouring cell.
********************************/
void process_single_row (int i, int ncols, int nrows, char * inboard, char * outboard, int LDA) {
    
    int j;
    
    for (j = 0; j < ncols; j++)
    {
        char cell = BOARD(inboard,i,j);
        int alive = cell >> 4;
        
        //if cell is dead and has 3 neighbouring cells
        if (!alive) {
            if (cell == (char)0x03 ) {
                
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
                
                //set the cell in outboards to the correct state
                SET_ALIVE(BOARD(outboard,i,j));
                
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
                
            }
        } else if (alive) { //if cell is alive and needs to die
            if (cell <= (char)0x11 || cell >= (char)0x14) {
         
                const int inorth = mod (i-1, nrows);
                const int isouth = mod (i+1, nrows);
                const int jwest = mod (j-1, ncols);
                const int jeast = mod (j+1, ncols);
            
                
                SET_DEAD(BOARD(outboard,i,j));

                DECR_AT (outboard, inorth, jwest);
                DECR_AT (outboard, inorth, j);
                DECR_AT (outboard, inorth, jeast);
                DECR_AT (outboard, i, jwest);
                DECR_AT (outboard, i, jeast);
                DECR_AT (outboard, isouth, jwest);
                DECR_AT (outboard, isouth, j);
                DECR_AT (outboard, isouth, jeast);
               
            }
        }
    }


}


