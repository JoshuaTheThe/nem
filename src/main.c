#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"textbuffer.h"
#include"action.h"
#include"editor.h"

int
main(void){
/*	txtelement_t *te;
	size_t i;
	initbuffers();
	(void)createbuffer();
	for(i=0UL;i<9UL;++i){
		insertat('A'+i,0);
	}

	insertat('Z',(size_t)-1L);
	removeat((size_t)-1L);
	for(i=0;i<buffsize();++i){
		te=findat(i);
		if(te)
			printf("char.%ld is %c (%d, %p)\n", i, te->val, te->val, te);
		else
			printf("not found: %ld\n", i);
	}*/

	initeditor();
	initbuffers();
	(void)createbuffer();
	editorredraw();
	
	while(editorrunning()){
		editor();
	}

	cleanupbuffers();
	cleanupactions();
	cleanupeditor();
	return(0);
}

