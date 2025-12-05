#ifndef ACTION_H
#define ACTION_H

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<stddef.h>
#include<unistd.h>
#include<string.h>
#include<dlfcn.h>

#include"textbuffer.h"

#define MAX_SEQUENCE_LENGTH (32UL)

typedef
enum{
	HOOK_NONE,
	HOOK_DELETE,
} hook_t;

typedef
struct _ACTION_{
        /**
         * Linked list previous && next
         * pointers.
         */
	struct _ACTION_ *prv;
	struct _ACTION_ *nxt;

        /**
         * The desired sequence for it's execution, and it's length.
	 * As-well as a hash, for performance.
         */
	char sequence[MAX_SEQUENCE_LENGTH];
	size_t hash;
	size_t sequnencelen;

        /**
         * Unique sequentially (originally)
         * assigned Identifier.
         */
	size_t id;

	/* Return true upon an error, takes itself
	 * and the working buffer as parameters. */
	bool (*upon)(struct _ACTION_ *, txtbuffer_t*, const char **argv, size_t argc);
	bool (*hook)(struct _ACTION_ *, hook_t hook);
	bool (*init)(struct _ACTION_ *);

	/* Data that the upon and hook functions can use,
	 * use the hook function's delete hook to cleanup properly for
	 * nested data. (as in, this points to a pointer,
         * this itself is freed) if it is non-NULL. */
	void *static_data;

	/**
	 * The pointer to the Shared Object
	 */
	void *dl_handle;
} action_t;

/* Management */

/* Append an action to the linked list */
void appendaction(action_t *action);

/* Create/Delete a/<none> new/an empty/<none> action */
action_t *createaction(char *object_path);
void removeaction(action_t *action);

/* Find an action by it's id */
action_t *findaction(size_t id);
/* Same, but by it's hash */
action_t *findactionhash(size_t hash);

/* Setup an action */
bool setaction(action_t *action, char *sequence, size_t len);

/* Delete them all */
void cleanupactions(void);

/* En-act */
bool action(action_t *act, txtbuffer_t *buff, const char **argv, size_t argc);

size_t hashsequence(char *seq, size_t len);

#endif

