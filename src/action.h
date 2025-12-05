#ifndef ACTION_H
#define ACTION_H

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<stddef.h>
#include<unistd.h>
#include<string.h>

#include"textbuffer.h"

#define MAX_SEQUENCE_LENGTH (32UL)

typedef enum{
	HOOK_NONE,
	HOOK_DELETE,
} hook_t;

typedef
struct _ACTION_{
	struct _ACTION_ *prv;
	struct _ACTION_ *nxt;
	char sequence[MAX_SEQUENCE_LENGTH];
	size_t sequnencelen;
	size_t id;

	/* Return true upon an error, takes itself
	 * and the working buffer as parameters. */
	bool (*upon)(struct _ACTION_ *, txtbuffer_t*);
	bool (*hook)(struct _ACTION_ *, hook_t hook);

	/* Data that the upon function can use,
	 * i reccomend using it so that we can cleanup properly;
	 * use the hook function's delete hook to cleanup properly for
	 * nested data. */
	void *static_data;
} action_t;

/* Management */
void appendaction(action_t *action);
action_t *createaction(void);
void removeaction(action_t *action);
action_t *findaction(size_t id);
bool setaction(action_t *action, char *sequence, size_t len);
void cleanupactions(void);

/* En-act */
bool action(action_t *act, txtbuffer_t *buff);

#endif

