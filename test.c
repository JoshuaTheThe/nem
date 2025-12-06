int main(int argc, char **argv){
	printf("Hello, World!\n");
}

int foo(int hashed, void (*fn)(void)){
	printf("created with hash of %d, at %p\n",hashed,fn);
}