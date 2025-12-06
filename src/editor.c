#include"editor.h"

static char rawcommand[MAX_COMMAND_SIZE];
static char command[MAX_COMMAND_SIZE][MAX_COMMAND_ARGS];
static int  argument_count;
static bool running;
static size_t cursorpos;
static size_t line=1;
static int fg,bg;
static txtelement_t *elem,*lineelem;

#ifndef _WIN32

int
min(int a, int b){
	return(a<b)?a:b;
}

#endif

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
			fflush(stdin);
			//tcflush(STDIN_FILENO, TCIFLUSH);
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

#ifdef _WIN32

bool
getpos(int *x, int *y){
	HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if(GetConsoleScreenBufferInfo(hConsole,&csbi)){
		*x = csbi.dwCursorPosition.X;
		*y = csbi.dwCursorPosition.Y;
		return false;
	}else{
		*x=*y=0;
		return true;
	}
}

#else

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

#endif

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
	fclose(fp);
	return false;
}

bool
upon_open(action_t *this, txtbuffer_t *buf, const char **argv, size_t argc){
	(void)this;
	char *path=strdup("test.txt");
	if(argc>1){
		free(path);
		path=strdup(argv[1]);
	}if(!path){
		return true;
	}
	FILE *fp = fopen(path, "r");
	if(!fp){
		free(path);
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
	line=1;
	elem=lineelem=buf->dat;
	editorredraw();
	movecurs(1, getheight());
	printf("argc=%ld",argc);
	free(path);
	fclose(fp);
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

#ifdef _WIN32

int
getwidth(void){
	HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (hConsole == INVALID_HANDLE_VALUE){
		return 80;
	}if(!GetConsoleScreenBufferInfo(hConsole,&csbi)){
		return 80;
	}

	return csbi.srWindow.Right - csbi.srWindow.Left + 1;
}

int
getheight(void){
	HANDLE hConsole=GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (hConsole == INVALID_HANDLE_VALUE){
		return 25;
	}if(!GetConsoleScreenBufferInfo(hConsole,&csbi)){
		return 25;
	}

	return csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
}

#else

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

#endif

void
movecurs(int x, int y){
	printf("\033[%d;%dH", y, x);
}

void
initeditor(void){
	/**
	 * Set-up Terminal
	 */
	fg=37;
	bg=44;
	elem=NULL;
	lineelem=NULL;
#ifndef _WIN32
	struct termios termios_p;
	tcgetattr(STDIN_FILENO, &termios_p);
	termios_p.c_lflag &= ~(ICANON |  ECHO);
	termios_p.c_iflag &= ~(IXON | IXOFF);
	tcsetattr(STDIN_FILENO, TCSANOW, &termios_p);
	
	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);
#endif

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
#ifndef _WIN32	
	struct termios termios_p;
	tcgetattr(STDIN_FILENO, &termios_p);
	termios_p.c_lflag |= (ICANON |  ECHO);
	termios_p.c_iflag |= (IXON | IXOFF);
	tcsetattr(STDIN_FILENO, TCSANOW, &termios_p);

	int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
#endif
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

#ifndef _WIN32

bool
kbhit(void){
	struct timeval tv = {0, 0};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	return select(1, &fds, NULL, NULL, &tv) > 0;
}

#endif

int
getlinelength(int line_num){
	txtbuffer_t *buff = grabbuffer();
	if(buff->len == 0) return 0;
	int current_line = 1;
	size_t line_start_idx = 0;
	txtelement_t *el=buff->dat;
	if(!el)return 0;

	for(size_t i = 0; i < buff->len; i++){
		if(!el){
			break;
		}if(current_line == line_num){
			line_start_idx = i;
			break;
		}if(el->val == '\n'){
			current_line++;
		}
		el=el->nxt;
	}
	if(current_line < line_num)
		return 0;
	int length = 0;
	for(size_t i = line_start_idx; i < buff->len; i++){
		if(!el)
			break;
		char c = el->val;
		el=el->nxt;
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
	if (!buff)return;
	int screen_x = 1, screen_y = 1;
	int screen_width = getwidth();
	txtelement_t *base = buff->dat;
	if (!base)return;
	size_t counter = 0;
	while(base && counter++ < idx){
		char c = base->val;
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
		base=base->nxt;
	}
	*x = screen_x;
	*y = screen_y;
}

size_t
getidxfrompos(int target_x, int target_y){
	txtbuffer_t *buff = grabbuffer();
	if (!buff)return 0;
	txtelement_t *el = buff->dat;
	if (!el)return 0;
	int screen_x = 1, screen_y = 1;
	int screen_width = getwidth();
	size_t idx = 0;
	while(el&&idx < buff->len && (screen_y < target_y || (screen_y == target_y && screen_x < target_x))){
		char c = el->val;
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
		el=el->nxt;
		idx++;
	}
	return idx;
}

void
setcol(int foreground, int background){
	printf("\033[%d;%dm", foreground, background);
}

void
editorredraw(void){
	setcol(fg,bg);
	size_t height=getheight();
	int x=1,y=line;
	size_t startpos=0;

	int csx=1,csy=1;
	getposfromidx(cursorpos, &csx, &csy);
	if(line>1&&cursorpos>0)
		startpos = getidxfrompos(1,line);
	printf("\033[2J");
	printf("\033[H");

	txtbuffer_t *buff = grabbuffer();
	txtelement_t *ele = lineelem;
	if (!buff)return;
	for(size_t i=startpos;i<buff->len;++i){
		if(!ele)
			break;
		char chr=ele->val;
		if(chr=='\n'){
			y+=1;
		}if((y-line)>=height-2){
			break;
		}
		++x;
		printf("%c", chr);
		ele=ele->nxt;
	}

	while((y-line)<height-3){
		printf("\n~");
		y+=1;
	}

	movecurs(1, height);
	printf("Going to %d,%d, startpos=%ld",csx,csy, startpos);
	movecurs(csx,csy-(line-1));
	fflush(stdout);
}

void
editor(void){
	if (!kbhit()){return;}
	char chr=getchar();
	if((chr&0x1f)==chr){
		int ctrlkey = chr;
		int x=1,y=1;
		fflush(stdin);
		getposfromidx(cursorpos, &x, &y);
		if(ctrlkey==('A'&0x1f)&&cursorpos>0){
			--cursorpos;
			getposfromidx(cursorpos, &x, &y);
	 		movecurs(x,y-(line-1));
			if(elem&&elem->prv)
				elem=elem->prv;
			fflush(stdout);
			return;
		}else if(ctrlkey==('D'&0x1f)&&cursorpos<grabbuffer()->len){
			++cursorpos;
			getposfromidx(cursorpos, &x, &y);
			movecurs(x,y-(line-1));
			if(elem&&elem->nxt)
				elem=elem->nxt;
			fflush(stdout);
			return;
		} else if(ctrlkey==('W'&0x1f)&&y>1){
			y -= 1;
			cursorpos = getidxfrompos(min(x, getlinelength(y)), y);
			elem = findat(cursorpos);
			
			while (((size_t)y-line)==0){
				line-=1;
			}
			if(line > 1){
				int length = getlinelength(line);
				for(int i = 0; i <= length; ++i)
					if(lineelem){
						lineelem=lineelem->prv;
					}
			}
			else lineelem=grabbuffer()->dat;
			editorredraw();
			getposfromidx(cursorpos, &x, &y);
	 		movecurs(x,y-(line-1));
			fflush(stdout);
			return;
		} else if(ctrlkey==('S'&0x1f)){
			y += 1;
			cursorpos = getidxfrompos(min(x, getlinelength(y)), y);
			elem = findat(cursorpos);
			while (((size_t)y-line)>(size_t)TEXT_HEIGHT-1){
				line+=1;
			}
			if(line > 1){
				int length = getlinelength(line);
				printf("line len = %d", length);
				for(int i = 0; i <= length; ++i)
					if(lineelem){
						lineelem=lineelem->nxt;
					}
			}
			else lineelem=grabbuffer()->dat;
			editorredraw();
			getposfromidx(cursorpos, &x, &y);
	 		movecurs(x,y-(line-1));
			fflush(stdout);
			return;
		}else if(ctrlkey==('E'&0x1f)){
			return;
		}else if(ctrlkey==('Q'&0x1f)){
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
		movecurs(x,y);
		fflush(stdout);
	}else if((chr!='\b'&&chr!=127&&(chr>=' '&&chr<='~'))||chr=='\n'||chr=='\t'){
		txtelement_t *el = insertat(chr,cursorpos++);
		if(el)
			elem=el;
		if(!lineelem&&el){
			lineelem=elem;
		}
		int x,y;
		getposfromidx(cursorpos,&x,&y);
		editorredraw();
		movecurs(1, TERM_HEIGHT);
		printf("added %d,%c",chr,chr == '\n' ? '\0' : '\n' ? '\0' : chr);
		movecurs(x,y-(line-1));
		fflush(stdout);
	}else if(cursorpos>0&&(chr=='\b'||chr==127)&&elem){
		char prev_char = 0;
		if(elem && elem->prv){
			prev_char=elem->prv->val;
			elem=elem->prv;
		}

		if(prev_char == '\n'){
			--cursorpos;
			if (elem==lineelem){
				int length = getlinelength(line-1);
				for(int i = 0; i < length; ++i)
					if(lineelem)
						lineelem=lineelem->prv;
			}if(elem->nxt){
				removechar(elem->nxt, grabbuffer());
			}else{
				removechar(elem, grabbuffer());
				elem=NULL;
			}
			editorredraw();
			fflush(stdout);
		}else{
			//redrawfrom(cursorpos - 16);
			--cursorpos;
			if(elem->nxt){
				removechar(elem->nxt, grabbuffer());
			}else{
				removechar(elem, grabbuffer());
				elem=NULL;
			}
			editorredraw();
			fflush(stdout);
		}
	}
}
