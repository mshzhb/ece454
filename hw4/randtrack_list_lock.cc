
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "defs.h"
#include "hash_list_lock.h"

#define SAMPLES_TO_COLLECT   10000000
#define RAND_NUM_UPPER_BOUND   100000
#define NUM_SEED_STREAMS            4

/* 
 * ECE454 Students: 
 * Please fill in the following team struct 
 */
team_t team = {
    "Super Unicorn",             /* Team name */

    "Michael Law",               /* First member full name */
    "997376343",                 /* First member student number */
    "m.law@mail.utoronto.ca",    /* First member email address */

    "Chi Yeung Jonathan Ng",      /* Second member full name */
    "997836141",                  /* Second member student number */
    "jonathancy.ng@utoronto.ca"   /* Second member email address */
};

unsigned num_threads;
unsigned samples_to_skip;

class sample;

class sample {
  unsigned my_key;
 public:
  sample *next;
  unsigned count;

  sample(unsigned the_key){my_key = the_key; count = 0;};
  unsigned key(){return my_key;}
  void print(FILE *f){printf("%d %d\n",my_key,count);}
};

class param {
  public:
    int start;
    int end;
};

// This instantiates an empty hash table
// it is a C++ template, which means we define the types for
// the element and key value here: element is "class sample" and
// key value is "unsigned".  
hash<sample,unsigned> h;

void * process_stream (void *ptr);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int main (int argc, char* argv[]){

  // Print out team information
  
  printf( "Team Name: %s\n", team.team );
  printf( "\n" );
  printf( "Student 1 Name: %s\n", team.name1 );
  printf( "Student 1 Student Number: %s\n", team.number1 );
  printf( "Student 1 Email: %s\n", team.email1 );
  printf( "\n" );
  printf( "Student 2 Name: %s\n", team.name2 );
  printf( "Student 2 Student Number: %s\n", team.number2 );
  printf( "Student 2 Email: %s\n", team.email2 );
  printf( "\n" );
  

  // Parse program arguments
  if (argc != 3){
    printf("Usage: %s <num_threads> <samples_to_skip>\n", argv[0]);
    exit(1);  
  }
  sscanf(argv[1], " %d", &num_threads); // not used in this single-threaded version
  sscanf(argv[2], " %d", &samples_to_skip);

  // initialize a 16K-entry (2**14) hash of empty lists
  h.setup(14);


  int start = 0;
  int i, end, incr;

  switch (num_threads) {
    case 1:
      end = 4;
      incr = 0;
      break;
    case 2:
      end = 2;
      incr = 2;
      break;
    case 4:
      end = 1;
      incr = 1;
      break;
    default:
      return 0;
      break;
  }

  pthread_t thread[num_threads];
  param *p = new param[num_threads];

  for (i=0; i< num_threads; i++){
    p[i].start = start;
    p[i].end = end;
    pthread_create(&thread[i], NULL, process_stream, (void*) &p[i]);
    start+=incr;
    end+=incr;

  }
  for (i=0; i<num_threads; i++){
    pthread_join(thread[i],NULL);
  }

  // print a list of the frequency of all samples
  h.print();
  h.cleanup();


}


void * process_stream (void *ptr) {

  param *p = (param*) ptr;

  int i,j,k;
  int rnum, key;
  sample * s;
  // process streams starting with different initial numbers
  for (i=p->start; i<p->end; i++){
    rnum = i;

    // collect a number of samples
    for (j=0; j<SAMPLES_TO_COLLECT; j++){

      // skip a number of samples
      for (k=0; k<samples_to_skip; k++){
        rnum = rand_r((unsigned int*)&rnum);
      }

      // force the sample to be within the range of 0..RAND_NUM_UPPER_BOUND-1
      key = rnum % RAND_NUM_UPPER_BOUND;

      //h.lookup_and_insert_if_absent(key);

      //pthread_mutex_lock(&mutex);

      // if this sample has not been counted before
      if (!(s = h.lookup(key))){
        // insert a new element for it into the hash table
        s = new sample(key);
        h.insert(s);
      }

      // increment the count for the sample
      h.lock_list(key);
      s->count++;
      h.unlock_list(key);
      
      //pthread_mutex_unlock(&mutex);
    }
  }

}


