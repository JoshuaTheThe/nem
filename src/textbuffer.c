#include"textbuffer.h"

txtbuffer_t *working_buffer, *buffers;
bool just_created;

/**
 * Buffer Management
 */

size_t
buffsize(void){
	return(working_buffer->len);
}

void
appendbuffer(txtbuffer_t *buff){
	if(!buff){return;}
	if(!buffers){
		buffers=buff;
		working_buffer=buff;
		memset(buff,0,sizeof(*buff));
		just_created=true;
		return;
	}

	txtbuffer_t *tmp=buffers->nxt;
	buffers->nxt=buff;
	buff->nxt=tmp;
	buff->prv=buffers;
	if(tmp){
		tmp->prv=buff;
	}

	if(just_created){
		buffers->prv=buff;
		buff->nxt=buffers;
	}
}

txtbuffer_t *
createbuffer(void){
	txtbuffer_t *newbuff=calloc(1,sizeof(txtbuffer_t));
	if(!newbuff){
		return(txtbuffer_t *)NULL;
	}
	appendbuffer(newbuff);
	return(newbuff);
}

void
initbuffers(void){
	working_buffer=(txtbuffer_t*)NULL;
	buffers=(txtbuffer_t*)NULL;
	just_created=false;
}

void
focusbuffer(txtbuffer_t *buff){
	working_buffer=buff;
}

void
focusprev(void){
	if(!working_buffer||!working_buffer->prv)return;
	working_buffer=working_buffer->prv;
}

void
focusnext(void){
	if(!working_buffer||!working_buffer->nxt)return;
	working_buffer=working_buffer->nxt;
}

txtbuffer_t *
grabbuffer(void){
	return working_buffer;
}

void
cleanupelement(txtelement_t *elem){
	if(!elem)return;
	memset(elem,0,sizeof(*elem));
	free(elem);
}

void
cleanupbuffer(txtbuffer_t *buff){
	if(!buff)return;
	txtelement_t *curr = buff->dat;
	txtelement_t *prev = (txtelement_t *)NULL;
	while(curr){
		cleanupelement(prev);
		prev=curr;
		curr=curr->nxt;
	}
	cleanupelement(prev);
	memset(buff,0,sizeof(*buff));
	free(buff);
}

void
cleanupbuffers(void){
	txtbuffer_t *buff = buffers;
	txtbuffer_t *prev = (txtbuffer_t *)NULL;
	while(buff){
		cleanupbuffer(prev);
		prev=buff;
		buff=buff->nxt;
	}
	cleanupbuffer(prev);
	buffers=(txtbuffer_t*)NULL;
	working_buffer=(txtbuffer_t*)NULL;
}

/**
 * Buffer's Content's Management
 */

txtelement_t *
findatin(txtbuffer_t *buff, size_t at){
	if(!buff)return(txtelement_t*)(NULL);
	txtelement_t *ele=buff->dat;
	while(at-- && ele){
		ele=ele->nxt;
	}

	return(ele);
}

void
removeatin(txtbuffer_t *buff, size_t at){
	txtelement_t *p=findatin(buff, at);
	if(at==(size_t)-1L){
		removeatin(buff,buff->len-1);
		return;
	}if(!buff||!p){
		return;
	}if(p->nxt){
		p->nxt->prv=p->prv;
	}if(p->prv){
		p->prv->nxt=p->nxt;
	}if(buff->dat==p){
		buff->dat=p->nxt;
	}
	memset(p,0,sizeof(*p));
	free(p);
	buff->len-=1;
}

void
insertatin(txtbuffer_t *buff, wchar ch, size_t at){
	if(!buff)return;
	if(at==(size_t)-1L){
		insertatin(buff,ch,buff->len);
		return;
	}
	txtelement_t *new=calloc(1,sizeof(*new));
	txtelement_t *nxt=findatin(buff, at);
	new->val=ch;
	buff->len+=1;
	new->nxt=nxt;
	if(!buff->dat){
		buff->dat=new;
		return;
	}if(at==buff->len-1){
		txtelement_t *prv=findatin(buff, at-1);
		prv->nxt=new;
		new->prv=prv;
		return;
	}else if(nxt){
		new->prv=nxt->prv;
		nxt->prv=new;
	}if(nxt&&new->prv){
		new->prv->nxt=new;
	}if(at==0){
		buff->dat=new;
	}
}

void
insertat(wchar ch, size_t at){
	insertatin(working_buffer,ch,at);
}

void
removeat(size_t at){
	removeatin(working_buffer,at);
}

txtelement_t *
findat(size_t at){
	return(findatin(working_buffer,at));
}

