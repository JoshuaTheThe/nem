md obj
md bin
gcc "src/main.c" -o "obj/main.o" -Wall -Wextra -c -g
gcc "src/textbuffer.c" -o "obj/textbuffer.o" -Wall -Wextra -c -g
gcc "src/action.c" -o "obj/action.o" -Wall -Wextra -c -g
gcc "src/editor.c" -o "obj/editor.o" -Wall -Wextra -c -g
gcc "obj/main.o" "obj/textbuffer.o" "obj/editor.o" "obj/action.o" -o "bin/nem"
