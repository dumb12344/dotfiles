run: build
	./install

build:
	gcc -o install install.c -lncurses -lpanel -lmenu