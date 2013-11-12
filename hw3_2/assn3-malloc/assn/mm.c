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
/*************************************************************************
 * Basic Constants and Macros
 * You are not required to use these macros but may find them helpful.
*************************************************************************/
#define WSIZE       sizeof(void *)            /* word size (bytes) */
#define DSIZE       (2 * WSIZE)            /* doubleword size (bytes) */
#define CHUNKSIZE   (1<<7)      /* initial heap size (bytes) */

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

/* free list macros */

#define PREV(bp)        ((char *)(bp))
#define SUCC(bp)        ((char *)(bp) + WSIZE)

#define getNext(bp)     GET(SUCC(bp))
#define getPrev(bp)     GET(PREV(bp))

#define putNext(bp, dest) PUT(SUCC(bp),dest) 
#define putPrev(bp, dest) PUT(PREV(bp),dest)

void* heap_listp = NULL;
void* flhead = NULL; //free list head pointer
void* fltail = NULL; //free list tail pointer

int counter = 0;

/**********************************************************
 * mm_init
 * Initialize the heap, including "allocation" of the
 * prologue and epilogue
 **********************************************************/
int mm_init(void)
{
    printf("mm_init\n");
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
        return -1;
    
    PUT(heap_listp, 0);                         // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));   // prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));   // prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));    // epilogue header
    heap_listp += DSIZE;

    /* reset */
    flhead = NULL; 
    fltail = NULL;
    counter = 0;

    print_heap();
    return 0;
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
    //printf(" coalesce %p ",bp);
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {       /* Case 1 */
        //printf(" 1\n");
        addToFront(bp);
        return bp;
    }

    else if (prev_alloc && !next_alloc) { /* Case 2 */
        //printf(" 2\n");

        rmFromFreeList(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));

        addToFront(bp);
        return (bp);
    }

    else if (!prev_alloc && next_alloc) { /* Case 3 */
        //printf(" 3\n");

        rmFromFreeList(PREV_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));

        addToFront(PREV_BLKP(bp));
        return (PREV_BLKP(bp));
    }

    else {            /* Case 4 */
        //printf(" 4\n");

        rmFromFreeList(PREV_BLKP(bp));
        rmFromFreeList(NEXT_BLKP(bp));

        size += GET_SIZE(HDRP(PREV_BLKP(bp)))  +
            GET_SIZE(FTRP(NEXT_BLKP(bp)))  ;
        PUT(HDRP(PREV_BLKP(bp)), PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size,0));

        addToFront(PREV_BLKP(bp));
        return (PREV_BLKP(bp));

    }
}

/**********************************************************
 * rmFromFreeList
 * This function will remove the free block from the free list
 **********************************************************/
void rmFromFreeList(void *bp)
{
    //printf(" rm ");
    if ((getNext(bp) == NULL) && (getPrev(bp)== NULL)){
        //printf(" 1!!!");
        //printf("%p %p %p \n", getNext(bp), getPrev(bp), bp);
        
        flhead = NULL;
    }
    else if (bp == flhead && getPrev(bp) == NULL) {
        //printf(" 2");
        flhead = getNext(bp);
        if (flhead != NULL){
            putPrev(flhead, NULL);
        }
        putPrev(bp, NULL);
        putNext(bp, NULL);
        
    }
    else if (getNext(bp) == NULL){
        //printf(" 3");
        void * prev = getPrev(bp);
        if (prev != NULL) {
            putNext(prev,NULL);
        }
        putNext(bp, NULL);
        putPrev(bp, NULL);
        
    }
    else {
        //printf(" 4");
        void * prev = getPrev(bp);
        void * next = getNext(bp);
        putPrev(bp, NULL);
        putNext(bp, NULL);

        putNext(prev, next);
        putPrev(next, prev);
    }
    //printf("\n");
}

/**********************************************************
 * addToFront
 * This function will add to the front of the free list
 **********************************************************/
void addToFront(void *bp)
{
    //printf(" addToFront\n");
    if (flhead == NULL) {
        flhead = bp;
        putNext(flhead, NULL);
        putPrev(flhead, NULL);
        return;
    }
    putNext(bp, flhead);
    putPrev(bp, NULL);
    putPrev(flhead, bp);
    flhead = bp;
    

    
}


/**********************************************************
 * extend_heap
 * Extend the heap by "words" words, maintaining alignment
 * requirements of course. Free the former epilogue block
 * and reallocate its new header
 **********************************************************/
void *extend_heap(size_t words)
{
    //printf(" extend_heap %zu\n",words*WSIZE);
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

    /* Initialize free list pointers */
    putNext(bp, NULL);
    putPrev(bp, NULL);

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
    //printf("find_fit %zu",asize);
    void *bp;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp))))
        {
            return bp;
        }
    }
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
    //printf("find %zu",asize);

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
    //print_free_list();
    //printf("---place\n");


    size_t min_size = DSIZE+DSIZE;
    size_t free_size = GET_SIZE(HDRP(bp));
    size_t bsize;

    rmFromFreeList(bp);

    if (free_size >= asize+min_size) {
        //printf(" and split");
        //pack asize
        //printf(" asize %zu", asize);
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));

        //pack bsize
        bsize = free_size - asize;
        //printf(" bsize %zu \n", bsize);
        void * free_bp = NEXT_BLKP(bp);
        PUT(HDRP(free_bp), PACK(bsize, 0));
        PUT(FTRP(free_bp), PACK(bsize, 0));

        //add bsize to free list
        addToFront(free_bp);
        
    }
    else {
        //printf(" and done");
        //no splitting
        PUT(HDRP(bp), PACK(free_size, 1));
        PUT(FTRP(bp), PACK(free_size, 1));
    }

    //print_free_list();
}

/**********************************************************
 * mm_free
 * Free the block and coalesce with neighbouring blocks
 **********************************************************/
void mm_free(void *bp)
{
    
    //printf("\n%d mm_free %p\n",counter++, bp);
    if(bp == NULL){
      return;
    }
    size_t size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(size,0));
    PUT(FTRP(bp), PACK(size,0));

    putNext(bp, NULL);
    putPrev(bp, NULL);

    coalesce(bp);
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
    //print_free_list();
    //printf("\n%d mm_malloc ",counter++);
    

    size_t asize; /* adjusted block size */
    size_t extendsize; /* amount to extend heap if no fit */
    char * bp;

    /* Ignore spurious requests */
    if (size == 0)
        return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
        asize = 2 * DSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE-1))/ DSIZE);

    /* Search the free list for a fit */
    if ((bp = find_fit_free_list(asize)) != NULL) {
        //printf(" success");
        place(bp, asize);
        return bp;
    }
    //printf("fail");

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
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
    //printf("\n%d mm_realloc ",counter++);
    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0){
      mm_free(ptr);
      return NULL;
    }
    /* If oldptr is NULL, then this is just malloc. */
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
  return 1;
}
/**********************************************************
 * mm_check_freeblocks
 * Check if number of free blocks in free list is
 * equal to number of free blocks in heap
 *********************************************************/
int mm_check_freeblocks(void){
    void *bp;
    size_t heap_size = 0;
    int count_heap = 0;
    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (GET_ALLOC(HDRP(bp)) == 0) {
            count_heap ++;
        }
    }

    int count_freelist = 0;
    
    bp = flhead;
    if (bp != NULL) {
        while (bp!= NULL){
            count_freelist ++;
            bp = getNext(bp);
        }
    }
    printf(" count_heap: %d count_freelist: %d",count_heap, count_freelist);
    
    return 1;
}
/**********************************************************
 * print_heap
 * Prints the blocks within the heap
 *********************************************************/
void print_heap(){
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

/**********************************************************
 * print_block
 * print block information
 *********************************************************/
void print_block(void* bp) {
    printf("%p | allocated: %1lu | size %6u | %6u words | %p\n ",
            bp, GET_ALLOC(HDRP(bp)), GET_SIZE(HDRP(bp)),GET_SIZE(FTRP(bp)),FTRP(bp));       
}

/**********************************************************
 * print_free_list
 * print the nodes in the free list
 *********************************************************/
void print_free_list () {
    printf("\npfl----------------------------------------------------------\n");

    void *bp = flhead;

    if (bp == NULL) {
        printf(" NULL");
        printf("----------------------------------------------------------\n");
        return;
    }
    size_t size;
    while (bp!= NULL){
        size = GET_SIZE(HDRP(bp));
        printf(" size %zu | %p ",size, bp);
        //printf("%zu ",size);
        bp = getNext(bp);
    }
    printf("\n----------------------------------------------------------\n");
    mm_check_freeblocks();
}
