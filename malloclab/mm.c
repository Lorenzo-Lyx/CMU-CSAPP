/* @Filename 
 * mm_implicit.c
 * 
 * @Desc 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Lorenzo Lyx",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))


/* @Desc Basic constants and macors 
 */
#define WSIZE        4        // Word and header/footer size (bytes).
#define DSIZE        8        // Double word size (bytes).
#define CHUNKSIZE    (1<<12)  // The default size for expanding the heap (bytes).

#define MAX(x, y)    ( (x) > (y) ? (x) : (y) )

/* @Desc Manipulating the headers and footers in the free list can be troublesome
 * because it demands extensive use of casting and pointer arithmetic. It is helpful
 * to define a small set of macros for accessing and traversing the free list.
 */
/* @Desc Combine a size and an allocate bit and return a value that can be stored in 
 * a header or footer */
#define PACK(size, allocate_bit)   ( (size) | (allocate_bit) )

/* @Desc read and return the word referenced by argument {p} 
 * @Params {p} is typically a (void *) pointer.
 */
#define GET(p)      (*(unsigned int  *)(p))

/* @Desc set the word referenced by argument {p} to {value} 
 * @Param {p} is typically a (void *) pointer.
 */
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* @Desc: The GET_SIZE and GET_ALLOC macros return the size and allocate bit, 
 * respectively, from a header or footer at address {p}.
 * @Explain: the format of block is shown as Figure 9.39. The size of block is always
 * the multiple of 8.
 * @Attention: size = block_size + header_size(1 WSIZE) + footer_size(1 WSIZE)
 */
#define GET_SIZE(p)     ( GET(p) & ~0x7 )
#define GET_ALLOC(p)    ( GET(p) & 0x1 )

/* @Desc Given @var{payload_pointer}, compute address of its header and footer.
 * @Params {payload_pointer} points to the first payload byte.
 * @Attention @Def{FTRP} depends on @Def{HDRP}.
 */
#define HDRP(payload_pointer) \
					( (char *)(payload_pointer) - WSIZE )
#define FTRP(payload_pointer) \
					( (char *)(payload_pointer) + GET_SIZE(HDRP(payload_pointer)) - DSIZE )

/* @Desc Given @var{payload_pointer}, compute address of next and previous blocks.
 * @Attention @Def{NEXT_BLKP} depends on its header.
 * and @Def{PREV_BLKP} depends on previous block's footer.
 */
#define NEXT_BLKP(payload_pointer)  \
					( (char *)(payload_pointer) + GET_SIZE( ((char *)(payload_pointer)-WSIZE) ) ) 
#define PREV_BLKP(payload_pointer)  \
					( (char *)(payload_pointer) - GET_SIZE( ((char *)(payload_pointer)-DSIZE) ) )


static char* heap_listp = NULL;


// #define DEBUG_OPEN 1
#define COMMA	,
#ifdef DEBUG_OPEN
#define LOG_RECORD(info)	log_record(__func__, __LINE__, info)
#else
#define LOG_RECORD(info)	
#endif



/**
 * @Desc 
 * record the process of program in @File{logfile.txt} if @Micor{DEBUG_OPEN} is defined.
 * @Idea 
 * The implement is not perfect because @File{logfile.txt} will not be closen.
*/
static inline void log_record(const char *function_name, const int line_number, const char *info_format, ...){
	static FILE *log_file = NULL;
	static char info_buffer[1024];
	log_file = log_file == NULL ? fopen("logfile.txt", "w") : log_file;
	// @Explain genarate log infomation
	sprintf(info_buffer, "@Func{%s}@Line{%d}:\t%s\n", function_name, line_number, info_format);
	va_list params;
	// @Explain put the parameter address after @Param{info} in @Var{params}.
	va_start(params, info_format);
	vfprintf(log_file, info_buffer, params); 
	fflush(log_file);
	va_end(params); 
	return;
}



void mm_check();



/** 
 * @Desc 
 * The @func{coalesce} is a straightforward implementation of the four cases
 * in section <simple implementation details>, The free list format we have chosen-
 * with its prologue and epilogue blocks that are always marked as allocated-allows
 * us to ignore the potentially troublesome edge conditions where the requested block
 * @Var{payload_pointer} is at the beginning or end of the heap.
*/
static void *coalesce(void *payload_pointer) {
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(payload_pointer)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(payload_pointer)));
	size_t size = GET_SIZE(HDRP(payload_pointer));
    if ( prev_alloc && !next_alloc ){  // case 2
		size += GET_SIZE( HDRP(NEXT_BLKP(payload_pointer)) );
		PUT(HDRP(payload_pointer), PACK(size, 0));
		/* @Explain 
		 * #define FTRP(payload_pointer)        
		 *       ( (char *)(payload_pointer) + GET_SIZE(HDRP(payload_pointer)) - DSIZE )
		 */
		PUT(FTRP(payload_pointer), PACK(size, 0));
	} else if ( !prev_alloc && next_alloc ){  // case 3
		size += GET_SIZE( HDRP(PREV_BLKP(payload_pointer)) );
		PUT(FTRP(payload_pointer), PACK(size, 0));
		PUT(HDRP(PREV_BLKP(payload_pointer)), PACK(size, 0));
		payload_pointer= PREV_BLKP(payload_pointer);
	} else if ( !prev_alloc && !next_alloc ) {                                  // case 4
		size += (GET_SIZE( HDRP(PREV_BLKP(payload_pointer)) ) 
			+ GET_SIZE( HDRP(NEXT_BLKP(payload_pointer)) ));
		PUT(HDRP(PREV_BLKP(payload_pointer)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(payload_pointer)), PACK(size, 0));
		payload_pointer = PREV_BLKP(payload_pointer);
	}
	return payload_pointer;
}


/**
 * @Desc 
 * The @func{extend_heap} is invoked in two different circumstances:
 * (1) when the heap is initialized;
 * (2) when @func{mm_malloc} is unable to find a suitable fit.
 * To maintain alignment, @func{extend_heap} rounds up the requested size to the
 * nearest multiple of 2 words(8 bytes) and then requests the additional heap space 
 * from the memory system.
 * @Param{words} is the number of words, for examples, we want to extend the heap by
 * 8*WSIZE bytes, then we only need to let @param{words} is 8.
*/
static void *extend_heap(size_t words) {
	LOG_RECORD("Beginning.");
	char *payload_pointer;
	size_t size;
	// Allocate an even number of words to maintain aignment.
	size = (words % 2) ? (words+1)*WSIZE : words*WSIZE;
	
	if( (long)(payload_pointer = mem_sbrk(size)) == -1 )
		return NULL;
	
	// Initialize free block header, footer and the epilogue header.
	/* @Explain @var{payload_pointer} points to the begining of new area.
	 * (lookat@func{mem_sbrk}.
	 *     @val{payload_pointer-WSIZE} is the epilogue header. 
	 * we change the epilogue header to  the header of the new block 
	 * and change the next block to epilogue block.
	 */
	PUT(HDRP(payload_pointer), PACK(size, 0));        // Free block header.
	PUT(FTRP(payload_pointer), PACK(size, 0));        // Free block footer.
	PUT(HDRP(NEXT_BLKP(payload_pointer)), PACK(0, 1)); // New epilogue header.
	// LOG_RECORD("The address of new area is %p."COMMA HDRP(payload_pointer));
	// LOG_RECORD("the size of new area is %d."COMMA GET_SIZE(HDRP(payload_pointer)));

	// @Explain Coalesce if the previous block was free. 
	return coalesce(payload_pointer);
} 

/** @Desc Create the initial empty heap.
 * 	@Explain initialize four words to create the empty free list. It then calls the 
 *		@func{extend_heap}, which extends the heap by @CONT{CHUNKSIZE} bytes and creates
 * 	the initial free block. At this point, the allocator is initialized and ready to 
 * 	accept allocate and free requests from the application.
 *     	@svar{heap_listp} is a special block pointer, it always points to the prologue
 * 	header. Of course, It also can point to the next block.
*/
int mm_init(void) {
	// create the initial empty heap.
	LOG_RECORD("Beginning.");
	if( (heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1 )
		return -1;
	PUT(heap_listp, 0);                             // Alignment padding
	PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));    // Prologue header
	PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));    // Prologue footer
	PUT(heap_listp + (3*WSIZE), PACK(0,1));          // Epilogue header
	
	heap_listp += (2*WSIZE);
	// extend the empty heap with a free block of @CONT{CHUNKSIZE} bytes
	if( extend_heap(CHUNKSIZE/WSIZE) == NULL )
		return -1;
	// LOG_RECORD("The address of the first free block is %p"COMMA HDRP(NEXT_BLKP(heap_listp)));
	// LOG_RECORD("the size of first free block is %d"COMMA GET_SIZE(HDRP(NEXT_BLKP(heap_listp))));
	LOG_RECORD("@Return correctly, 0.");
	return 0;
}


/** 
 * 	@Desc It performs a first-fit search of the implicit free list.
 * 	@Callback @var{heap_listp} can always point to the prologue block to implement
 * 	first-fit policy.
 * 	@Callback @param{adjusted_size) is block size that @func{mm_malloc} adjusts requested 
 * 	size(payload) to include overhead(header&footer) and alignment requirement.(8-byte) 
 */
static void *find_fit(size_t adjusted_size) {
	LOG_RECORD("Beginning.");
	char* payload_pointer = NEXT_BLKP(heap_listp);
	/* @Idea 
	 * @Code{static char* payload_pointer = heap_listp} performs a next-fit search.
	 */
		
	size_t block_size = 0;
	/* @Explain 
	 * check until the end.
	 */
	while( (block_size=GET_SIZE(HDRP(payload_pointer))) > 0 ) {
		if( !GET_ALLOC(HDRP(payload_pointer)) && block_size >= adjusted_size ){
			LOG_RECORD("@Return, find a Fit!");
			return payload_pointer;
		}
		payload_pointer = NEXT_BLKP(payload_pointer);
	}
	LOG_RECORD("@Return NULL.");
	return NULL;
}

/* @Desc It places the requested block at the beginning of the free block, splitting
 * only if the size of the reminder would euqal or exceed the minimum block size.
 */
static void place(void *payload_pointer, size_t adjusted_size){
	LOG_RECORD("Beginning.");
	size_t payload_size = GET_SIZE(HDRP(payload_pointer));
		
	/* @Explain the sequence of the following code is important, because @Def{FTRP}
	 * depends on @Def{HERP}.
	 */
	if( payload_size-adjusted_size >= 2*DSIZE ){
		PUT(FTRP(payload_pointer), PACK(payload_size-adjusted_size, 0));
		PUT(HDRP(payload_pointer), PACK(adjusted_size, 1));
		PUT(HDRP(NEXT_BLKP(payload_pointer)), PACK(payload_size-adjusted_size, 0));
		PUT(FTRP(payload_pointer), PACK(adjusted_size, 1));
	} else {
		PUT(HDRP(payload_pointer), PACK(payload_size, 1));
		PUT(FTRP(payload_pointer), PACK(payload_size, 1));
	}
	LOG_RECORD("@Return.");
	return;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
	LOG_RECORD("Beginning.");
	// @Explain Ignore supurious requests
	if( size == 0 ) {
		return NULL;
	}

	/* @Explain Adjust block size to include overhead(header&footer) and
	 * alignment requirement.(8-byte)
	 */
	size_t adjusted_size = ALIGN((size+DSIZE));
		
	// @Explain Search the free list for a fit.
	char *payload_pointer = NULL;
	if( (payload_pointer=find_fit(adjusted_size)) != NULL) {
		place(payload_pointer, adjusted_size);
		LOG_RECORD("It is correct, %p"COMMA payload_pointer);
		return payload_pointer;
	}
	/* @Explain If the allocator cannot find a fit, it extends the heap with a new free
	 * block, place the requested block in the new free block.
	 */
	size_t extend_size = MAX(adjusted_size, CHUNKSIZE);
	if( (payload_pointer=extend_heap(extend_size/WSIZE)) !=NULL ){
		place(payload_pointer, adjusted_size);
	}
	// @Explain Payload_pointer may be NULL.
	return payload_pointer;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *payload_pointer) {
	size_t size = GET_SIZE(HDRP(payload_pointer));

	PUT(HDRP(payload_pointer), PACK(size, 0));
	PUT(FTRP(payload_pointer), PACK(size, 0));
	coalesce(payload_pointer);

#ifdef DEBUG_OPEN
	mm_check();
#endif
	return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *payload_pointer, size_t new_size)
{
	if( payload_pointer == NULL ) {
		return mm_malloc(new_size);
	} else if( new_size == 0 ) {
        mm_free(payload_pointer);
        return NULL;
    }
    
    size_t adjusted_size = ALIGN((new_size+DSIZE));
    /* @Explain In the case that the requested size is less than original size, 
     * return immediately if @Param{new_size}/(the size of the block) > 0.5,
     * or free the remainder otherwise.
     */
    // @Desc @var{the_size} is the size of original block.
    // @Lable RequestSizeLess
    size_t the_size = GET_SIZE(HDRP(payload_pointer));  
    if( adjusted_size <= the_size ) {
        if( (double)adjusted_size/the_size < 0.5 ) {
            PUT(HDRP(payload_pointer), PACK(adjusted_size, 1));
            PUT(FTRP(payload_pointer), PACK(adjusted_size, 1));
            void *next = NEXT_BLKP(payload_pointer);
            PUT(HDRP(next), PACK(the_size-adjusted_size, 0));
            PUT(FTRP(next), PACK(the_size-adjusted_size, 0));
            coalesce(next);
        }
        return payload_pointer;
    }

    void *prev_payload = PREV_BLKP(payload_pointer);
    void *next_payload = NEXT_BLKP(payload_pointer);

    // @Desc @var{alloc_case} is (prev_alloc<<1 | next_alloc)
    size_t alloc_case = (GET_ALLOC(HDRP(prev_payload)) << 1) | GET_ALLOC(HDRP(next_payload));

    size_t prev_size = GET_SIZE(HDRP(prev_payload));
    size_t next_size = GET_SIZE(HDRP(next_payload));

    void *new_payload_pointer = NULL;
    size_t copy_size = the_size - DSIZE;

    /* @Explain The following code may be cofusing.
     * First, the case 0(00) is written outside of @switch in the reason that in case 1, case 2 and
     * case 3, space need to be reallocated if total size is less than @var{adjusted_size}. In other
     * words, the code of case 0 is used together by other cases.
     * Second, noticing the totoal size may be too large to requested size, we need use the code in 
     * @Label{RequestSizeLess}. It is implemented by calling @func{mm_realloc} again.
     */
    switch(alloc_case) {
    case 0: // @Desc 00, Both blocks are free.
        if ( next_size+the_size+prev_size >= adjusted_size ) {
            PUT(FTRP(next_payload), PACK(next_size+the_size+prev_size, 1));
            PUT(HDRP(prev_payload), PACK(next_size+the_size+prev_size, 1));
            memmove(prev_payload, payload_pointer, copy_size);
            return mm_realloc(prev_payload, adjusted_size-DSIZE);
        } else { // @Explain else go to case 3
            break;
        }
    case 1: // @Desc 01, Only previous block is free.
        if( prev_size+the_size >= adjusted_size ) {
            PUT(FTRP(payload_pointer), PACK(prev_size+the_size, 1));
            PUT(HDRP(prev_payload), PACK(prev_size+the_size, 1));
            memmove(prev_payload, payload_pointer, copy_size);
            return mm_realloc(prev_payload, adjusted_size-DSIZE);
        } else { // @Explain else go to case 3
            break;
        }

    case 2: // @Desc 10, Only next block is free.
        if( next_size+the_size >= adjusted_size ) {
            PUT(HDRP(payload_pointer), PACK(next_size+the_size, 1));
            PUT(FTRP(next_payload), PACK(next_size+the_size, 1));
            return mm_realloc(payload_pointer, adjusted_size-DSIZE);
        } else { // @Explain else go to case 3
            break;
        }
    }
    // @Desc case 3: 11, Both blocks is allocated, we can only malloc again.
    new_payload_pointer = mm_malloc(new_size);
    if( new_payload_pointer == NULL ) {
        return NULL;
    }
    memmove(new_payload_pointer, payload_pointer, copy_size);
    mm_free(payload_pointer);

	return new_payload_pointer;
}


void mm_check() {
	char* payload_pointer = NEXT_BLKP(heap_listp);
	size_t block_size = 0;
	while( (block_size=GET_SIZE(HDRP(payload_pointer))) > 0 ) {
		if(GET_ALLOC(HDRP(payload_pointer))) {
			LOG_RECORD("\t\ta\t\t%d"COMMA GET_SIZE(HDRP(payload_pointer)));
		} else {
			LOG_RECORD("\t\tf\t\t%d"COMMA GET_SIZE(HDRP(payload_pointer)));
		}
		payload_pointer = NEXT_BLKP(payload_pointer);
	}
    LOG_RECORD("\n");
	return;
}











