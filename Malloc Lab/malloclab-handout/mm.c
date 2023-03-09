/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
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

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
	/* Team name */
	"cxk",
	/* First member's full name */
	"ctrl",
	/* First member's email address */
	"cxk@xhz.zyntm",
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

static char *heap_listp;

static void *find_fit(size_t size);
static void *extend_heap(size_t size);
static void place(void *ptr, size_t size);
static void *coalesce(void *ptr);

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12)
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc))
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))


static void *find_fit(size_t size) {
	for (char *tmp = heap_listp; GET_SIZE(HDRP(tmp)) > 0; tmp = NEXT_BLKP(tmp)) {
		if (!GET_ALLOC(HDRP(tmp)) && GET_SIZE(HDRP(tmp)) >= size) {
			return tmp;
		}
	}
	return NULL;
}

static void *extend_heap(size_t size) {
	if (size <= 0) {
		return NULL;
	}
	void *ptr;
	size_t asize = size % 2 ? (size + 1) * WSIZE : size * WSIZE;
	if ((ptr = mem_sbrk(asize)) == (void *) -1) {
		return NULL;
	}
	PUT(HDRP(ptr), PACK(asize, 0));
	PUT(FTRP(ptr), PACK(asize, 0));
	PUT(HDRP(NEXT_BLKP(ptr)), PACK(0, 1));
	return coalesce(ptr);
}

static void place(void *ptr, size_t size) {
	size_t p_size = GET_SIZE(HDRP(ptr));
	if (p_size - size >= 2 * DSIZE) {
		PUT(HDRP(ptr), PACK(size, 1));
		PUT(FTRP(ptr), PACK(size, 1));
		ptr = NEXT_BLKP(ptr);
		PUT(HDRP(ptr), PACK(p_size - size, 0));
		PUT(FTRP(ptr), PACK(p_size - size, 0));
	} else {
		PUT(HDRP(ptr), PACK(p_size, 1));
		PUT(FTRP(ptr), PACK(p_size, 1));
	}
}

static void *coalesce(void *ptr) {
	size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
	size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
	size_t size = GET_SIZE(HDRP(ptr));
	if (prev_alloc && next_alloc) {
		return ptr;
	} else if (!prev_alloc && next_alloc) {
		size += GET_SIZE(HDRP(PREV_BLKP(ptr)));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
		PUT(FTRP(ptr), PACK(size, 0));
		return PREV_BLKP(ptr);
	} else if (prev_alloc && !next_alloc) {
		size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
		PUT(HDRP(ptr), PACK(size, 0));
		PUT(FTRP(ptr), PACK(size, 0));
		return ptr;
	} else {
		size += GET_SIZE(HDRP(NEXT_BLKP(ptr))) + GET_SIZE(HDRP(PREV_BLKP(ptr)));
		PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0));
		PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0));
		return PREV_BLKP(ptr);
	}
	return ptr;
}

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
	if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *) -1) {
		return -1;
	}
	PUT(heap_listp, 0);
	PUT(heap_listp + WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 2 * WSIZE, PACK(DSIZE, 1));
	PUT(heap_listp + 3 * WSIZE, PACK(0, 1));
	heap_listp += 2 * WSIZE;
	if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
		return -1;
	}
	return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
	if (size <= 0) {
		return NULL;
	}
	void *ptr;
	size_t asize;
	if (size <= DSIZE) {
		asize = 2 * DSIZE;
	} else {
		asize = DSIZE * ((size + DSIZE + (DSIZE - 1)) / DSIZE);
	}
	if ((ptr = find_fit(asize)) != NULL) {
		place(ptr, asize);
		return ptr;
	}
	size_t ex_size = MAX(CHUNKSIZE, asize);
	if ((ptr = extend_heap(ex_size / WSIZE)) != NULL) {
		place(ptr, asize);
		return ptr;
	}
	return NULL;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
	if (ptr != NULL) {
		PUT(HDRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
		PUT(FTRP(ptr), PACK(GET_SIZE(HDRP(ptr)), 0));
		coalesce(ptr);
	}
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
	void *oldptr = ptr;
	void *newptr;
	size_t copySize;

	newptr = mm_malloc(size);
	if (newptr == NULL)
		return NULL;
	size = GET_SIZE(HDRP(oldptr));
	copySize = GET_SIZE(HDRP(newptr));
	if (size < copySize)
		copySize = size;
	memcpy(newptr, oldptr, copySize - WSIZE);
	mm_free(oldptr);
	return newptr;
}














