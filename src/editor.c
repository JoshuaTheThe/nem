#include"editor.h"

static char rawcommand[MAX_COMMAND_SIZE];
static char command[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS];
static int  argument_count;
static bool running;
static size_t cursorpos;

void
getcommand(void){
	int i = 0;
	char chr;
	
	memset(rawcommand, 0, MAX_COMMAND_SIZE);
	movecurs(1, getheight()-1);	
	printf(": ");
	fflush(stdout);
	
	while(i < MAX_COMMAND_SIZE-1){
		while(!kbhit()){;}
		chr = getchar();
		
		switch(chr){
		case '\n':
		case '\r':
		case COMMAND_CHARACTER:
			fflush(stdout);
			tcflush(STDIN_FILENO, TCIFLUSH);
			goto end;
		case 127:
		case '\b':
			if(i < 1)
				continue;
			--i;
			printf("\b \b");
			fflush(stdout);
			rawcommand[i] = '\0';
			break;
		default:
			printf("%c", chr);
			fflush(stdout);
			rawcommand[i] = chr;
			++i;
			break;
		}
	}
end:
	rawcommand[i] = '\0';
	return;
}

bool
getpos(int *x, int *y){
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	tcflush(STDIN_FILENO, TCIFLUSH);
	printf("\033[6n");
	fflush(stdout);
	char response[32];
	size_t i=0;
	char c;
	struct timeval tv = {0, 50000};
	fd_set fds;
	while(i < sizeof(response) - 1){
		FD_ZERO(&fds);
		FD_SET(STDIN_FILENO, &fds);
		int ret = select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv);
		if(ret <= 0){
			break;
		}
		
		if(read(STDIN_FILENO, &c, 1) != 1){
			break;
		}
		
		response[i++] = c;
		if(c == 'R')
			break;
	}
	response[i] = '\0';
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	if(i < 4 || response[0] != '\033' || response[1] != '['){
		return false;
	}if(sscanf(response + 2, "%d;%d", y, x) != 2){
		return false;
	}
	return true;
}

void
parsecommand(void){
	char *x=strtok(&rawcommand[0]," ");
	argument_count=0;
	while(x&&x<&rawcommand[MAX_COMMAND_SIZE]&&argument_count<MAX_COMMAND_ARGS){
		strncpy(command[argument_count],x,MAX_COMMAND_SIZE-1);
		command[argument_count][MAX_COMMAND_SIZE - 1] = '\0';
		x=strtok(NULL," ");
		argument_count += 1;
	}
}

bool
hook_generic(action_t *this, hook_t hook){
	(void)this;
	(void)hook;
	return(false);
}

bool
upon_redraw(action_t *this, txtbuffer_t *buf, const char **argv, size_t argc){
	(void)this;
	(void)buf;
	(void)argv;
	(void)argc;
        editorredraw();
        movecurs(1, getheight());
        printf("Redrew");
	return(false);
}

bool
upon_save(action_t *this, txtbuffer_t *buf, const char **argv, size_t argc){
	(void)this;
	char *path="test.txt";
	if(argc>1){
		path=strdup(argv[1]);
	}
	FILE *fp = fopen(path, "w");
	if(!fp){
		return true;
	}
	txtelement_t *elem = buf->dat;
	while(elem){
		fprintf(fp,"%c",elem->val);
		elem=elem->nxt;
	}
	movecurs(1, getheight());
	printf("argc=%ld",argc);
	free(path);
	return false;
}

bool
upon_open(action_t *this, txtbuffer_t *buf, const char **argv, size_t argc){
	(void)this;
	char *path="test.txt";
	if(argc>1){
		path=strdup(argv[1]);
	}
	FILE *fp = fopen(path, "r");
	if(!fp){
		return true;
	}
	emptybuffer(buf);
	char chr=fgetc(fp);
	cursorpos=0;
	while(chr!=EOF){
		insertatin(buf,chr,cursorpos++);
		chr=fgetc(fp);
	}
	cursorpos=0;
	editorredraw();
	movecurs(1, getheight());
	printf("argc=%ld",argc);
	free(path);
	return false;
}

bool
upon_exit(action_t *this, txtbuffer_t *buf, const char **argv, size_t argc){
	(void)this;
	(void)buf;
	(void)argv;
	(void)argc;
	running = false;
	return(false);
}

bool
init_generic(action_t *this){
	(void)this;
	return(false);
}

void
signal(int signum){
	switch(signum){
	case 2:
		cleanupeditor();
		running=false;
		break;
	}
}

int
getheight(void){
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0){
		return w.ws_row;
	}else{
		return 25;
	}
}

int
getwidth(void){
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0){
		return w.ws_col;
	}else{
		return 80;
	}
}

void
movecurs(int x, int y){
	printf("\033[%d;%dH", y, x);
}

void
initeditor(void){
	/**
	 * Set-up Terminal
	 */
	struct termios termios_p;
	tcgetattr(STDIN_FILENO, &termios_p);
	termios_p.c_lflag &= ~(ICANON |  ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &termios_p);
	
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

	/**
	 * Set-up Initial commands
	 */
	action_t *exi = createaction(NULL); /* exit */
	action_t *red = createaction(NULL); /* redraw */
	action_t *sav = createaction(NULL); /* save */
	action_t *ope = createaction(NULL); /* open */
	red->hook = hook_generic;
	red->upon = upon_redraw;
	red->init = init_generic;
	red->init(red);
	strncpy(red->sequence, "R", MAX_SEQUENCE_LENGTH);
	red->hash = hashsequence("R",1);

	sav->hook = hook_generic;
	sav->init = init_generic;
	sav->upon = upon_save;
	strncpy(sav->sequence, "s", MAX_SEQUENCE_LENGTH);
	sav->hash = hashsequence("s",1);
	sav->init(sav);

	ope->hook = hook_generic;
	ope->init = init_generic;
	ope->upon = upon_open;
	strncpy(ope->sequence, "o", MAX_SEQUENCE_LENGTH);
	ope->hash = hashsequence("o",1);
	ope->init(ope);

	exi->hook = hook_generic;
	exi->upon = upon_exit;
	exi->init = init_generic;
	exi->init(exi);
	strncpy(exi->sequence, "x", MAX_SEQUENCE_LENGTH);
	exi->hash = hashsequence("x",1);
	running = true;
	cursorpos=0;
	editorredraw();
}

void
cleanupeditor(void){
	/**
	 * Restore Terminal
	 */
	
	struct termios termios_p;
	tcgetattr(STDIN_FILENO, &termios_p);
	termios_p.c_lflag |= (ICANON |  ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &termios_p);

	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
}

bool
editorrunning(void){
	return(running);
}

void
execcommand(void){
	size_t hash = hashsequence(command[0], strnlen(command[0],MAX_COMMAND_SIZE));
	action_t *act = findactionhash(hash);

	const char* args[argument_count + 1];
	for(int i=0;i<argument_count;i++){
		args[i] = command[i];
	}
	action(act, grabbuffer(), args, argument_count);
}

bool
kbhit(void){
	struct timeval tv = {0, 0};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	return select(1, &fds, NULL, NULL, &tv) > 0;
}

int
getlinelength(int line_num){
	txtbuffer_t *buff = grabbuffer();
	if(buff->len == 0) return 0;
	int current_line = 1;
	size_t line_start_idx = 0;
	for(size_t i = 0; i < buff->len; i++){
		if(current_line == line_num){
			line_start_idx = i;
			break;
		}if(findat(i)->val == '\n'){
			current_line++;
		}
	}
	if(current_line < line_num)
		return 0;
	int length = 0;
	for(size_t i = line_start_idx; i < buff->len; i++){
		char c = findat(i)->val;
		if(c == '\n'){
			break;
		}
		length++;
	}
	return length;
}

void
getposfromidx(size_t idx, int *x, int *y){
	txtbuffer_t *buff = grabbuffer();
	int screen_x = 1, screen_y = 1;
	int screen_width = getwidth();
	for(size_t i = 0; i < idx && i < buff->len; i++){
		char c = findat(i)->val;
		if(c == '\n'){
			screen_y++;
			screen_x = 1;
		}else if(c == '\t'){
			screen_x = ((screen_x + 7) / 8) * 8 + 1;
			if(screen_x > screen_width){
				screen_x = screen_x - screen_width + 1;
				screen_y++;
			}
		}else{
			screen_x++;
			if(screen_x > screen_width){
				screen_x = 1;
				screen_y++;
			}
		}
	}
	*x = screen_x;
	*y = screen_y;
}

void
editorredraw(void){
	printf("\033[2J");
	printf("\033[H");
	fflush(stdout);

	txtbuffer_t *buff = grabbuffer();
	if (!buff)return;
	for(size_t i=0;i<buff->len;++i){
		printf("%c", findat(i)->val);
	}

	int x,y;
	getposfromidx(cursorpos, &x,&y);
	movecurs(x,y);
}

size_t
getidxfrompos(int target_x, int target_y){
	txtbuffer_t *buff = grabbuffer();
	int screen_x = 1, screen_y = 1;
	int screen_width = getwidth();
	size_t idx = 0;
	while(idx < buff->len && (screen_y < target_y || (screen_y == target_y && screen_x < target_x))){
		char c = findat(idx)->val;
		if(c == '\n'){
			screen_y++;
			screen_x = 1;
		}else if(c == '\t'){
			screen_x = ((screen_x + 7) / 8) * 8 + 1;
			if(screen_x > screen_width){
				screen_x = screen_x - screen_width + 1;
				screen_y++;
			}
		}else{
			screen_x++;
			if(screen_x > screen_width){
				screen_x = 1;
				screen_y++;
			}
		}
		idx++;
	}
	return idx;
}

void
redrawfrom(size_t idx){
	txtbuffer_t *buff = grabbuffer();
	int cursor_x, cursor_y;
	getpos(&cursor_x, &cursor_y);
	
	int start_x, start_y;
	getposfromidx(idx, &start_x, &start_y);
	movecurs(start_x, start_y);
	printf("\033[J");
	fflush(stdout);
	
	for(size_t i=idx;i<buff->len;i++){
		char c = findat(i)->val;
		printf("%c", c);
	}
	
	movecurs(cursor_x, cursor_y);
	fflush(stdout);
}

void
editor(void){
	if (!kbhit()){return;}
	char chr=getchar();
	if((chr&0x1f)==chr){
		int ctrlkey = chr;
		int x,y;
		if(ctrlkey==('A'&0x1f)&&cursorpos>0){
			getposfromidx(--cursorpos,&x,&y);
	 		movecurs(x,y);
			fflush(stdout);
			return;
		}if(ctrlkey==('D'&0x1f)&&cursorpos<grabbuffer()->len){
			getposfromidx(++cursorpos,&x,&y);
			movecurs(x,y);
			fflush(stdout);
			return;
		}
	}if(chr==COMMAND_CHARACTER){
		int x,y;
		getpos(&x,&y);
		getcommand();
		parsecommand();
		movecurs(1, getheight()-1);
		for (int i=0;i<getwidth();++i)
			printf(" ");
		fflush(stdout);
		execcommand();
		fflush(stdout);
		movecurs(x,y);
	}else if(chr!='\b'&&chr!=127){
		insertat(chr,cursorpos++);
		editorredraw();
		//redrawfrom(cursorpos-16);
		fflush(stdout);
	}else if(cursorpos>0){
		char prev_char = findat(cursorpos - 1)->val;
		if(prev_char == '\n'){
			int x, y;
			getpos(&x, &y);
			if(y > 1){
				movecurs(1, y - 1);
				movecurs(getlinelength(y - 1) + 1, y - 1);
			}
			//redrawfrom(cursorpos - 16);
			editorredraw();
			removeat(--cursorpos);
		}else{
			//redrawfrom(cursorpos - 16);
			editorredraw();
			removeat(--cursorpos);
			printf("\b \b");
			fflush(stdout);
		}
	}
}
