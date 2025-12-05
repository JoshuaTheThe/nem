#ifndef TXT_BUFF_H
#define TXT_BUFF_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<stddef.h>
#include<unistd.h>

typedef
unsigned short wchar;

typedef
struct _TXTELEMENT_{
        /**
         * Linked list previous && next
         * pointers.
         */
	struct _TXTELEMENT_ *prv;
	struct _TXTELEMENT_ *nxt;

        /**
         * The extended-ascii/ut8 (you decide idk)
         * value.
         */
	wchar		     val;
} txtelement_t;

typedef
struct _TXTBUFFER_{
        /**
         * Linked list previous && next
         * pointers.
         */
	struct _TXTBUFFER_ *prv;
	struct _TXTBUFFER_ *nxt;

        /**
         * The pointer to the first-
         * element, may be null.
         */
	struct _TXTELEMENT_ *dat;
	size_t len;
} txtbuffer_t;

/**
 * Buffers Management.
 */

/* Grab the size of the working buffer. */
size_t buffsize(void);

/* Append a buffer to the linked list. */
void appendbuffer(txtbuffer_t *buff);

/* Create a new buffer*/
txtbuffer_t *createbuffer(void);

/* Setup Initial state */
void initbuffers(void);

/* Set the working buffer, to the respective buffer. */
void focusbuffer(txtbuffer_t *buff);
void focusnext(void);
void focusprev(void);

/* Return the working buffer. */
txtbuffer_t *grabbuffer(void);

/* Remove all of the text from a buffer. */
void cleanupbuffer(txtbuffer_t *buff);

/* Delete all of the buffers. */
void cleanupbuffers(void);

/**
 * Content's.
 */

/* Specific Buffer. */

/* Find the element at the given index. */
txtelement_t *findatin(txtbuffer_t *buff, size_t at);

/* Remove the element at the given index. */
void removeatin(txtbuffer_t *buff, size_t at);

/* Insert a wchar at the given index. */
void insertatin(txtbuffer_t *buff, wchar ch, size_t at);

/* Current Buffer (do the same, but to the working buffer). */
txtelement_t *findat(size_t at);
void removeat(size_t at);
void insertat(wchar ch, size_t at);

#endif

