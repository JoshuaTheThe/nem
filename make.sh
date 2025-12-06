#! /bin/bash 
mkdir -p obj 
mkdir -p bin 
# Compile 
cc "src/main.c" -o "obj/main.o" -Wall -Wextra -c -g 
cc "src/textbuffer.c" -o "obj/textbuffer.o" -Wall -Wextra -c -g 
cc "src/action.c" -o "obj/action.o" -Wall -Wextra -c -g 
cc "src/editor.c" -o "obj/editor.o" -Wall -Wextra -c -g 
# Link 
cc "obj/main.o" "obj/textbuffer.o" "obj/editor.o" "obj/action.o" -o "bin/nem" 
# Memory Leak test
if [ $# -ne 1 ]; then
	exit
fi
if [ "$1" == "-test" ]; then
	valgrind --leak-check=yes --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./bin/nem
fi
