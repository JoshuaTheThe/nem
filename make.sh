#! /bin/bash 
mkdir -p obj 
mkdir -p bin 
# Compile 
cc "src/main.c" -o "obj/main.o" -Wall -Wextra -c -g 
cc "src/textbuffer.c" -o "obj/textbuffer.o" -Wall -Wextra -c -g 
cc "src/action.c" -o "obj/action.o" -Wall -Wextra -c -g 
# Link 
cc "obj/main.o" "obj/textbuffer.o" "obj/action.o" -o "bin/nem" 