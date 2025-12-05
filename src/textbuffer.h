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
	struct _TXTELEMENT_ *prv;
	struct _TXTELEMENT_ *nxt;
	wchar		     val;
} txtelement_t;

typedef
struct _TXTBUFFER_{
	struct _TXTBUFFER_ *prv;
	struct _TXTBUFFER_ *nxt;
	struct _TXTELEMENT_ *dat;
	size_t len;
} txtbuffer_t;

/**
 * Buffers Management
 */

size_t buffsize(void);
void appendbuffer(txtbuffer_t *buff);
txtbuffer_t *createbuffer(void);
void initbuffers(void);
void focusbuffer(txtbuffer_t *buff);
void focusnext(void);
void focusprev(void);
txtbuffer_t *grabbuffer(void);

void cleanupbuffer(txtbuffer_t *buff);
void cleanupbuffers(void);

/**
 * Content's
 */

/* Specific Buffer */
txtelement_t *findatin(txtbuffer_t *buff, size_t at);
void removeatin(txtbuffer_t *buff, size_t at);
void insertatin(txtbuffer_t *buff, wchar ch, size_t at);

/* Current Buffer */
txtelement_t *findat(size_t at);
void removeat(size_t at);
void insertat(wchar ch, size_t at);

#endif

