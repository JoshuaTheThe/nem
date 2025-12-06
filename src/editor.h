#ifndef NEM_EDIT_H
#define NEM_EDIT_H

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdint.h>
#include<stdbool.h>
#include<stddef.h>
#include<unistd.h>

#ifdef _WIN32
	#include<conio.h>
	#include<windows.h>
#else
	#include<termios.h>
	#include<sys/ioctl.h>
#endif

#include<fcntl.h>
#include<math.h>
#include"action.h"
#include"textbuffer.h"

#define MAX_COMMAND_SIZE (512)
#define MAX_COMMAND_ARGS (8)
#define COMMAND_CHARACTER ('\e')
#define TERM_WIDTH (getwidth())
#define TERM_HEIGHT (getheight())
#define TEXT_HEIGHT (TERM_HEIGHT - 1)

void initeditor(void);
bool editorrunning(void);
void editor(void);
void cleanupeditor(void);

#ifndef _WIN32
	bool kbhit(void);
#endif

int getheight(void);
int getwidth(void);
void editorredraw(void);
void movecurs(int x, int y);
bool getpos(int *x, int *y);

#endif

