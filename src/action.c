#include"action.h"

action_t *actions;
size_t action_count;

/**
 * Registrating && Finding Actions by their ID.
 */
void
appendaction(action_t *action){
	if(!action){return;}
	if(!actions){
		actions=action;
		memset(action,0,sizeof(*action));
		action->id=action_count++;
		return;
	}

	action_t *tmp=actions->nxt;
	actions->nxt=action;
	action->nxt=tmp;
	action->prv=actions;

	if(tmp){
		tmp->prv=action;
	}
	action->id=action_count;
	action_count++;
}

action_t *
createaction(void){
	action_t *new=(action_t*)calloc(1,sizeof(*new));
	if(!new){
		return((action_t*)NULL);
	}
	appendaction(new);
	return new;
}

void
removeaction(action_t *action){
	if(!action)return;
	if(actions==action){
		actions=(action_t*)NULL;
	}if(action->prv){
		action->prv->nxt=action->nxt;
	}if(action->nxt){
		action->nxt->prv=action->prv;
	}
}

action_t *
findaction(size_t id){
	action_t *action=actions;
	while(action){
		if(action->id==id){
			return action;
		}
		action=action->nxt;
	}
	return((action_t*)NULL);
}

bool
setaction(action_t *action, char *sequence, size_t len){
	if(!action||!sequence){ return true; }
	if(len>=MAX_SEQUENCE_LENGTH){
		return true;
	}

	memcpy(action->sequence, sequence, len);
	return false;
}

void
cleanupact(action_t *act){
	if(!act)return;
	if(act->hook){
		bool res=act->hook(act,HOOK_DELETE);
		fprintf(stderr, "deleted %s\n",
				res ? "with errors" : "successfully");
	}if(act->static_data){
		free(act->static_data);
	}
	memset(act,0,sizeof(*act));
	free(act);
}

void
cleanupactions(void){
	action_t *act=actions;
	action_t *prv=(action_t*)NULL;
	while(act){
		cleanupact(prv);
		prv=act;
		act=act->nxt;
	}

	cleanupact(prv);
	actions=(action_t*)NULL;
}

/**
 * Action execution
 */

bool
action(action_t *act, txtbuffer_t *buff){
	if(!act)return true;
	bool res=act->upon(act, buff);
	return res;
}

