/*
 * This implementation replicates the implicit list implementation
 * provided in the textbook
 * "Computer Systems - A Programmer's Perspective"
 * Blocks are never coalesced or reused.
 * Realloc is implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Super Unicorn",
    /* First member's full name */
    "CHI YEUNG JONATHAN NG",
    /* First member's email address */
    "jonathancy.ng@utoronto.ca",
    /* Second member's full name (leave blank if none) */
    "MICHAEL LAW",
    /* Second member's email address (leave blank if none) */
    "m.law@mail.utoronto.ca"
};

#define DEBUG 3
/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)         /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */
#define OVERHEAD    DSIZE     /* overhead of header and footer (bytes) */

#define MAX(x,y) ((x) > (y)?(x) :(y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)          (*(uintptr_t *)(p))
#define PUT(p,val)      (*(uintptr_t *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~(DSIZE - 1))
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char *)(bp) - WSIZE)
#define FTRP(bp)        ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* alignment */
#define ALIGNMENT 16
/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0xf)

/* free list macros */

#define PREV(bp)        ((char *)(bp))
#define SUCC(bp)        ((char *)(bp) + WSIZE)

#define getNext(bp)     GET(SUCC(bp))
#define getPrev(bp)     GET(PREV(bp))

#define putNext(bp, dest) PUT(SUCC(bp),dest) 
#define putPrev(bp, dest) PUT(PREV(bp),dest)



/* print function definitions */
void print_heap();
void print_block(void* bp);
void * find_fit_free_list(size_t asize);
void reset_4_mm_init(void);
void print_free_list();

void* heap_listp = NULL;
void* flhead = NULL; //free list head pointer
void* fltail = NULL; //free list tail pointer

int counter = 0;
int mm_init_once = 0;
/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
 int mm_init(void)
 {
#if DEBUG >= 2
    printf("MM_INIT\n");
    if (mm_init_once) {
        mm_init_once = 0;
    } else {
        mm_init_once = 1;
    }
#endif
    reset_4_mm_init();

	// try to initialize memory to 4*WSIZE first
	if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) // out of memory, could not expand heap
         return -1;

	PUT(heap_listp, 0);                         // alignment padding
	PUT(heap_listp + (1 * WSIZE), PACK(OVERHEAD, 1));   // prologue header
	PUT(heap_listp + (2 * WSIZE), PACK(OVERHEAD, 1));   // prologue footer

	// epilogue block is an allocated block of size 0
	PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header

	// move the heap pointer to the payload of the "block" we just initialized
	heap_listp += DSIZE;

	print_heap();  

	return 0;
 }

 void reset_4_mm_init(void){

    flhead = NULL;
    fltail = NULL;
 }

/**********************************************************
 * coalesce
 * Covers the 4 cases discussed in the text:
 * - both neighbours are allocated
 * - the next block is available for coalescing
 * - the previous block is available for coalescing
 * - both neighbours are available for coalescing
 **********************************************************/
void *coalesce(void *bp)
{
#if DEBUG >= 3
    printf("coalesce c\n");
#endif

    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));


    void * prev;
    void * next; 
    void * fp;


/* free list setup*/
    if (prev_alloc && next_alloc) {       /* Case 1 */
        printf(" case 1 a|self|a\n");

        if (flhead == NULL) {
            flhead = bp;
            fltail = bp;
            printf(" flhead set to bp %p\n",bp);
            getPrev(bp) = NULL;
            getNext(bp) = NULL;

        }
        else {   
            printf(" moved to top of list.. bp %p\n",bp);
            getPrev(flhead) = bp;
            getNext(bp) = flhead;
            flhead = bp;
        }
        getPrev(bp) = NULL;
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */

        printf(" case 2 a|self|free\n");

        //printf("step0 base case\n");

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        

        if (flhead == NEXT_BLKP(bp)){
            prev = getPrev(flhead);
            next = getNext(flhead);
            getNext(bp) = next;
            getPrev(bp) = prev;
            return flhead;
        }

        //step 1: connect prev and next
        //printf("step1\n");
        fp = NEXT_BLKP(bp);
        prev = getPrev(fp);
        next = getNext(fp);

        
        if (prev != NULL){
            getNext(prev) = next;
        }
        if (next != NULL){
            getPrev(next) = prev;
        }


        //step 2: put in front
        //printf("step2\n");
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        getNext(bp) = flhead;
        getPrev(flhead) = bp;
        flhead = bp;
        getPrev(flhead) = NULL;
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3: a free block on the left*/
        printf("case 3 free|self|a for %p\n", bp);
        //print_heap();

        /*
        //implicit
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        return (PREV_BLKP(bp));
        */
        //printf("step0 base case\n");

        /* free list */
        //printf(" size before %zu\n",size);
        //printf(" foot address %p\n", ((char *)(bp) - DSIZE));
        //printf(" footer size %zu\n", GET_SIZE(((char *)(bp) - DSIZE)) );
        //printf(" adding size %zu\n", GET_SIZE(HDRP(PREV_BLKP(bp))));
        //printf(" the pointer is at %p\n", PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        //printf(" size after %zu\n",size);


        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        

        // base case: at fhead there
        if (flhead == PREV_BLKP(bp)) {
            return flhead; 
        }

        // step 1: connect prev and next
        //printf("step1..\n");
        prev = getPrev(PREV_BLKP(bp));
        next = getNext(PREV_BLKP(bp));


        if (prev != NULL) {
            getNext(prev) = next;
        }

        if (next != NULL){
            getPrev(next) = prev;
        }   

        // step 2: put newly free block at the front of the list
        //printf("step2..\n");
        fp = PREV_BLKP(bp);
        getNext(fp) = flhead;
        getPrev(flhead) = fp;
        flhead = fp;
        getPrev(flhead) = NULL;

        //printf("done!\n");
        return (PREV_BLKP(bp));


    }

    else {            /* Case 4 */ 
        /*
        printf("case4 free|self|free\n");
        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));


        return (PREV_BLKP(bp));
        */

        printf(" case4/1 a|self|a\n");

        if (flhead == NULL) {
            flhead = bp;
            fltail = bp;
            printf(" flhead set to bp %p\n",bp);
            getPrev(bp) = NULL;
            getNext(bp) = NULL;

        }
        else {   
            printf(" moved to top of list.. bp %p\n",bp);
            getPrev(flhead) = bp;
            getNext(bp) = flhead;
            flhead = bp;
            getPrev(flhead) = NULL;
        }
        getPrev(bp) = NULL;
        return bp;
    }



}

/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
#if DEBUG >= 3
    printf(" %zu words ", words);
#endif

    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignments */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ( (bp = mem_sbrk(size)) == (void *)-1 )
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));                // free block header
    PUT(FTRP(bp), PACK(size, 0));                // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));        // new epilogue header

    
    getNext(bp) = NULL;
    getPrev(bp) = NULL;

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}


/**********************************************************
 * find_fit
 * Traverse the heap searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit(size_t asize)
{
#if DEBUG >= 3
    printf("find_fit");
#endif

    void *bp;

	// step through the heap starting at the top of the heap_listp and increment by a block at a time
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
		// if the current block is not allocated and the size of the block is less than the size
		// that we want to allocate, then we return this block (first fit algorithm)
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            return bp;
        }
    }
	// if we get here, that means there were no unallocated blocks of >= asize
    return NULL;
}
/**********************************************************
 * find_fit_free_list
 * Traverse the free list searching for a block to fit asize
 * Return NULL if no free blocks can handle that size
 * Assumed that asize is aligned
 **********************************************************/
void * find_fit_free_list(size_t asize)
{
#if DEBUG >= 3
    printf("find_fit");
#endif

    void *bp = flhead;
    
    if (bp == NULL) return NULL;

    while (bp!= NULL){
        if ( asize <= GET_SIZE(HDRP(bp))) 
        {
            return bp;
        }
        bp = getNext(bp);
    }
    return NULL;
}

/**********************************************************
 * place
 * Mark the block as allocated
 **********************************************************/
void place(void* bp, size_t asize)
{
    
#if DEBUG >= 3
    printf(" ==place== ");
#endif

    // min_size is the smallest free block possible: header(wsize) + 2*WSIZE + footer(wsize)
    // aligned to DSIZE (2*WSIZE)
    size_t min_size = DSIZE+OVERHEAD;
	size_t free_size = GET_SIZE(HDRP(bp));

    size_t bsize = asize;
    uintptr_t * prev;
    uintptr_t * next; 
    if (free_size >= asize+min_size)
    {
        printf(" asize %zu", asize);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        
        
        bsize = free_size - asize;
        printf(" bsize %zu \n", bsize);
        void * free_bp = NEXT_BLKP(bp);
        PUT(HDRP(free_bp), PACK(bsize, 0));
        //printf("footer %p\n",FTRP(free_bp));
        //printf("bsize %zu\n",bsize);
        PUT(FTRP(free_bp), PACK(bsize, 0));
        //printf("bsize in footer %zu\n",GET_SIZE(FTRP(free_bp)));
        //print_heap();

        

        /* free list */
        if (flhead == fltail) { //one free block
            printf("breakpoint1\n");
            flhead = free_bp;
            fltail = free_bp;
            getNext(free_bp) = NULL;
            getPrev(free_bp) = NULL;
        }
        else if (bp == flhead) { //more than one free block and at head
            printf("breakpoint2\n");

            prev = getPrev(bp);
            next = getNext(bp);
            flhead = free_bp;
            getPrev(free_bp) = prev;
            getNext(free_bp) = next; 

            if (prev != NULL ) {
                getNext(prev) = free_bp;
            }
            if (next != NULL) {
                getPrev(next) = free_bp;
            }


        }
        else if (bp == fltail) {
            printf("breakpoint3\n");
            prev = getPrev(bp);
            next = getNext(bp);
            fltail = free_bp;
            getPrev(free_bp) = prev;
            getNext(free_bp) = next; 

            if (prev != NULL ) {
                getNext(prev) = free_bp;
            }
            if (next != NULL) {
                getPrev(next) = free_bp;
            }

        }
        else {
            printf("breakpoint4\n");
            prev = getPrev(bp);
            next = getNext(bp);
            getPrev(free_bp) = prev;
            getNext(free_bp) = next;


            if (prev != NULL ) {
                getNext(prev) = free_bp;
            }
            if (next != NULL) {
                getPrev(next) = free_bp;
            }
        }


    }
    else  //no need to split
    {

        printf(" no split\n");
        //if (counter >=  770) print_heap();
        PUT(HDRP(bp), PACK(free_size, 1));
        PUT(FTRP(bp), PACK(free_size, 1));
        /* free list */
        
        if (flhead == fltail) {
            printf(" 1..\n");    
            flhead = fltail = NULL;
        }
        else if (bp == flhead) {
            printf(" 2..\n");
            flhead = getNext(flhead);
            getPrev(flhead)= NULL;
        }
        else if (bp == fltail) {
            printf(" 3..\n");
            fltail = getPrev(fltail);
            putNext(fltail,NULL);
        }
        else {
            prev = getPrev(bp);
            next = getNext(bp);
            if (prev!=NULL){
                printf(" 4 get prev.....\n");
                printf("prev's %p\n",prev);
                getNext(prev) = next;    
            }
            printf(" 4.2.....\n");
            if (next!=NULL){
                getPrev(next) = prev;
            }
            printf(" 4.3..done\n");
        }

    }

}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{

#if DEBUG >= 3
    printf("\nmm_free\n");

#endif
    //printf(" list before freeing..\n");
    //print_free_list();

    if(bp == NULL){
        printf("null found..");
        return;
    }

    size_t size = GET_SIZE(HDRP(bp));
    printf(" size %zu\n", size);
    PUT(HDRP(bp), PACK(size,0));

    PUT(FTRP(bp), PACK(size,0));

    if (bp == flhead){
        if (getNext(flhead) != NULL ){
            flhead = getNext(flhead);
        }
        else {
            flhead = NULL;
        }
    }
    if (bp == fltail){
        if (getPrev(fltail) != NULL){
            fltail = getPrev(fltail);
        } 
        else {
            fltail = NULL;    
        }
        
    }

    coalesce(bp);

    //printf("list after freeing\n");
    //print_free_list();
}


/**********************************************************
 * mm_malloc
 * Allocate a block of size bytes.
 * The type of search is determined by find_fit
 * The decision of splitting the block, or not is determined
 *   in place(..)
 * If no block satisfies the request, the heap is extended
 **********************************************************/
void *mm_malloc(size_t size)
{
#if DEBUG >= 3
    //print_heap();
    printf("\n%d mm_malloc",counter++);
#endif

    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = DSIZE + OVERHEAD;
    else
        asize = DSIZE * ((size + (OVERHEAD) + (DSIZE-1))/ DSIZE);

#if DEBUG >= 3
    printf(" %zu ", asize);
#endif

    /* Search the free list for a fit */
    if ((bp = find_fit_free_list(asize)) != NULL) {
        printf(" success\n");
        place(bp, asize);
        return bp;
    }
    printf(" fail");
    /* No fit found. Get more memory and place the block */

    extendsize = MAX(asize, CHUNKSIZE);
    printf(" extend_heap by %zu B ", extendsize);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
        return NULL;
    place(bp, asize);
    return bp;

}

/**********************************************************
 * mm_realloc
 * Implemented simply in terms of mm_malloc and mm_free
 *********************************************************/
void *mm_realloc(void *ptr, size_t size)
{
#if DEBUG >= 3
    printf("mm_realloc\n");
#endif

    /* If size == 0 then this is just free, and we return NULL. */
    if (size == 0){
      mm_free(ptr);
      return NULL;
    }

    /* If old ptr is NULL, then this is just malloc. */
    if (ptr == NULL)
      return (mm_malloc(size));

    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
      return NULL;

    /* Copy the old data. */
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

/**********************************************************
 * mm_check
 * Check the consistency of the memory heap
 * Return nonzero if the heap is consistant.
 *********************************************************/
int mm_check(void){

#if DEBUG >= 3
    printf("mm_check\n");
#endif

    void *bp;

	// step through the heap starting at the top of the heap_listp and increment by a block at a time
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
		// print out each heap block
    }
	return 1;
}

/* prints the whole heap */
void print_heap(){

#if DEBUG >= 3
    printf("print_heap\n");
#endif

    void *bp;

	printf("----------------------------------------------------------\n");

	// step through the heap starting at the top of the heap_listp and increment by a block at a time

    size_t heap_size = 0;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
		// print out each heap block
        heap_size += GET_SIZE(HDRP(bp));
		print_block(bp);
    }

    printf("Total heap size: %zu words - %zu bytes\n",heap_size, heap_size*WSIZE);
	printf("----------------------------------------------------------\n");
}

/* prints a single block */
void print_block(void* bp) {
#if DEBUG >= 3
    //printf("print_block\n");
#endif

	//int allocated = GET_ALLOC(HDRP(bp));
	//int size = GET_SIZE(HDRP(bp));
	printf("%p | allocated: %1lu | size %6u | %6u words | %p\n ",
			bp, GET_ALLOC(HDRP(bp)), GET_SIZE(HDRP(bp)),GET_SIZE(FTRP(bp)),FTRP(bp));		

}

void print_free_list () {
    printf (" print_free_list----------");

    void *bp = flhead;

    if (bp == NULL) {
        printf(" nothing to print");
        printf ("--------------end of print free list\n");
        return;
    }

    while (bp!= NULL){

        size_t size = GET_SIZE(HDRP(bp));

        //printf(" size %zu | %p ",size, bp);
        printf("%zu ",size);

        bp = getNext(bp);
    }
    printf ("--------------end of print free list\n");
}






