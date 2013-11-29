#include "load.h"
#include <assert.h>
#include <stdlib.h>
#include "util.h"

#define BOARD( __board, __i, __j )  (__board[(__i) + nrows*(__j)])
#define IS_ALIVE(var) ((var) & (1<<(4))) //check if the 5th bit is set --> alive

#define INCR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] ++ )
#define DECR_AT(__board, __i, __j )  (__board[(__i) + nrows*(__j)] -- )

void initialize_neighbour_counts (char * board, const int nrows, const int ncols);


char*
make_board (const int nrows, const int ncols)
{
  char* board = NULL;
  int i;

  /* Allocate the board and fill in with 'Z' (instead of a number, so
     that it's easy to diagnose bugs */
  board = malloc (2 * nrows * ncols * sizeof (char));
  assert (board != NULL);
  for (i = 0; i < nrows * ncols; i++)
    board[i] = 'Z';

  return board;
}

static void
load_dimensions (FILE* input, int* nrows, int* ncols)
{
  int ngotten = 0;
  
  ngotten = fscanf (input, "P1\n%d %d\n", nrows, ncols);
  if (ngotten < 1)
    {
      fprintf (stderr, "*** Failed to read 'P1' and board dimensions ***\n");
      fclose (input);
      exit (EXIT_FAILURE);
    }
  if (*nrows < 1)
    {
      fprintf (stderr, "*** Number of rows %d must be positive! ***\n", *nrows);
      fclose (input);
      exit (EXIT_FAILURE);
    }
  if (*ncols < 1)
    {
      fprintf (stderr, "*** Number of cols %d must be positive! ***\n", *ncols);
      fclose (input);
      exit (EXIT_FAILURE);
    }
}

/**********************************
  -setting any alive cells to new represenation: 0x10
  -call initialize_neighbour_counts
**********************************/
char*
load_board_values (FILE* input, const int nrows, const int ncols)
{
  char* board = NULL;
  int ngotten = 0;
  int i = 0;

  /* Make a new board */
  board = make_board (nrows, ncols);

  /* Fill in the board with values from the input file */
  for (i = 0; i < nrows * ncols; i++)
  {
      ngotten = fscanf (input, "%c\n", &board[i]);
      if (ngotten < 1)
      {
        fprintf (stderr, "*** Ran out of input at item %d ***\n", i);
        fclose (input);
        exit (EXIT_FAILURE);  
      }
      else
      /* ASCII '0' is not zero; do the conversion */
      board[i] = board[i] - '0';
      
      if (board[i] == 0x01) {
        board[i] = board[i] << 4;        
      }


  }
  initialize_neighbour_counts(board, nrows, ncols);

  return board;
}

void
initialize_neighbour_counts (char * board, const int nrows, const int ncols){
  
  int i,j;
  for (i = 0; i < nrows; i++) {
    for (j = 0; j< ncols; j++) {
      printf("%02x\n",board[i+nrows*j]);


      /*
      if (IS_ALIVE(BOARD(board, i, j))) {

        const int inorth = mod (i-1, nrows);
        const int isouth = mod (i+1, nrows);
        const int jwest = mod (j-1, ncols);
        const int jeast = mod (j+1, ncols);

        INCR_AT (board, inorth, jwest);
        INCR_AT (board, inorth, j);
        INCR_AT (board, inorth, jeast);
        INCR_AT (board, i, jwest);
        INCR_AT (board, i, jeast);
        INCR_AT (board, isouth, jwest);
        INCR_AT (board, isouth, j);
        INCR_AT (board, isouth, jeast);

      }
      */
      
    }
  }
  for (i = 0; i < nrows; i++) {
    for (j = 0; j< ncols; j++) {


  
      if (IS_ALIVE(BOARD(board, i, j))) {

        const int inorth = mod (i-1, nrows);
        const int isouth = mod (i+1, nrows);
        const int jwest = mod (j-1, ncols);
        const int jeast = mod (j+1, ncols);

        INCR_AT (board, inorth, jwest);
        INCR_AT (board, inorth, j);
        INCR_AT (board, inorth, jeast);
        INCR_AT (board, i, jwest);
        INCR_AT (board, i, jeast);
        INCR_AT (board, isouth, jwest);
        INCR_AT (board, isouth, j);
        INCR_AT (board, isouth, jeast);

      }
      
      
    }
  }
  printf("-----\n");

  for (i = 0; i < nrows; i++) {
    for (j = 0; j< ncols; j++) {
      printf("%02x\n",board[i+nrows*j]);


      /*
      if (IS_ALIVE(BOARD(board, i, j))) {

        const int inorth = mod (i-1, nrows);
        const int isouth = mod (i+1, nrows);
        const int jwest = mod (j-1, ncols);
        const int jeast = mod (j+1, ncols);

        INCR_AT (board, inorth, jwest);
        INCR_AT (board, inorth, j);
        INCR_AT (board, inorth, jeast);
        INCR_AT (board, i, jwest);
        INCR_AT (board, i, jeast);
        INCR_AT (board, isouth, jwest);
        INCR_AT (board, isouth, j);
        INCR_AT (board, isouth, jeast);

      }
      */
      
    }
  }




}




char*
load_board (FILE* input, int* nrows, int* ncols)
{
  load_dimensions (input, nrows, ncols);
  return load_board_values (input, *nrows, *ncols);
}













